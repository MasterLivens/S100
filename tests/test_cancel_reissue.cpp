// IHO-S164-CANCEL-REISSUE — docs/cancel-reissue-contract.md rev 2.1.
// CTest: exchange_cancel_reissue. MUST NOT SKIP: a missing fixture is a FAIL.
#include "s100/exchange_set.hpp"
#include "s100/iso8211.hpp"
#include "s100/session.hpp"
#include "support/dataset_digest.hpp"
#include "support/sha256.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using s100::UpdateRejectCode;

#define CCHECK(cond)                                                                \
  do {                                                                              \
    if (!(cond)) {                                                                  \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";  \
      std::exit(1);                                                                 \
    }                                                                               \
  } while (0)

const char* name(UpdateRejectCode c) { return s100::update_reject_code_name(c); }
using s100test::dataset_digest;

// Values chosen by tools/gen_update_fixtures.py (seed 20261018).
constexpr std::uint32_t kInsertedRcid = 81;  // seq/.001 inserts FRID 81
constexpr std::uint32_t kDeletedRcid = 35;   // seq/.003 deletes FRID 35
const char* kCell = "101AA00DS0003";

fs::path s101_dir() { return fs::path(S100_SOURCE_DIR) / "testdata" / "s101"; }
fs::path upd_dir() { return s101_dir() / "updates"; }
fs::path base_path() { return s101_dir() / "101AA00DS0003.000"; }
fs::path cell(const char* dir, const char* ext) {
  return upd_dir() / dir / (std::string(kCell) + "." + ext);
}
fs::path seq(const char* ext) { return cell("seq", ext); }

const s100::Feature* by_rcid(const s100::Dataset& ds, std::uint32_t rcid) {
  for (const auto& f : ds.features)
    if (f.record_id == rcid) return &f;
  return nullptr;
}

std::map<UpdateRejectCode, int> g_seen;

/** fixtures.toml [[cancel_fixture]] path -> sha256. */
std::map<std::string, std::string> cancel_manifest() {
  std::map<std::string, std::string> m;
  std::ifstream in(s101_dir() / "fixtures.toml", std::ios::binary);
  CCHECK(in.good());
  std::string line, path;
  bool in_fixture = false;
  auto value = [](const std::string& l) {
    const auto a = l.find('"'), b = l.rfind('"');
    return a == std::string::npos || b <= a ? std::string() : l.substr(a + 1, b - a - 1);
  };
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.rfind("[", 0) == 0) {
      in_fixture = line == "[[cancel_fixture]]";
      path.clear();
      continue;
    }
    if (!in_fixture) continue;
    if (line.rfind("path = ", 0) == 0) path = value(line);
    if (line.rfind("sha256 = ", 0) == 0 && !path.empty()) m[path] = value(line);
  }
  return m;
}

/** Oracle 1: every fixture exists and matches its sha256. */
void check_fixture_set() {
  const std::vector<std::string> expected = {
      "updates/nodot/101AA00DS0003.000",
      "updates/nodot/101AA00DS0003.001",
      "updates/cancel/101AA00DS0003.001",
      "updates/cancel/101AA00DS0003.002",
      "updates/cancel/101AA00DS0003.003",
      "updates/cancel_bad/101AA00DS0003.002",
      "updates/cancel_wrong_name/101AA00DS0006.001",
      "updates/cancel_ooo/101AA00DS0003.001",
      "updates/cancel_ooo/101AA00DS0003.003",
      "updates/wrong_name/101AA00DS0006.001",
      "updates/reissue/101AA00DS0003.000",
      "updates/reissue/101AA00DS0003.001",
      "updates/reissue/101AA00DS0003.002",
      "updates/reissue/101AA00DS0003.003",
      "updates/newed/101AA00DS0003.000",
      "updates/newed/101AA00DS0003.001",
      "updates/newed/CATALOG.XML",
      "updates/pending/101AA00DS0003.001",
      "updates/session_replace/A/CATALOG.XML",
      "updates/session_replace/B/CATALOG.XML",
      "updates/session_replace/X/CATALOG.XML",
      "updates/session_replace/C/CATALOG.XML",
      "updates/dsed_bad/empty.001",
      "updates/dsed_bad/ed_x.001",
      "updates/dsed_bad/x.001",
      "updates/dsed_bad/ed_dot.001",
      "updates/dsed_bad/dot_upd.001"};
  const auto m = cancel_manifest();
  CCHECK(m.size() == expected.size());
  for (const auto& rel : expected) {
    const fs::path p = s101_dir() / fs::path(rel);
    if (!fs::exists(p)) {
      std::cerr << "FAIL missing fixture " << p << " (run tools/gen_update_fixtures.py)\n";
      std::exit(1);
    }
    auto it = m.find(rel);
    CCHECK(it != m.end());
    if (s100test::sha256_file_hex(p) != it->second) {
      std::cerr << "FAIL sha256 " << rel << "\n";
      std::exit(1);
    }
  }
  std::cout << "fixtures ok: " << expected.size() << " files, sha256 match fixtures.toml\n";
}

/** A DSID field body as encoded by S-101 (header + 7 UT + DSRD + DSLG/DSAB/DSED + DSTC). */
std::vector<std::uint8_t> dsid_field(const std::string& dsed) {
  std::string s;
  s += std::string("\x0a\x01\x00\x00\x00", 5);
  for (const char* ut : {"S-100 Part 10a", "5.2", "INT.IHO.S-101.2.0", "2.0", "2",
                         "101AA00DS0003.001", "title"})
    s += std::string(ut) + '\x1f';
  s += "20250304";
  s += std::string("EN") + '\x1f' + '\x1f' + dsed + '\x1f';
  s += std::string("\x0e\x12", 2);
  return std::vector<std::uint8_t>(s.begin(), s.end());
}

void dsed_unit_asserts() {
  struct Ok { const char* dsed; int ed, upd; };
  for (const Ok& c : {Ok{"0", 0, 0}, Ok{"9", 9, 0}, Ok{"4.20", 4, 20}, Ok{"9.0", 9, 0}}) {
    const auto id = s100::parse_s101_dsid(dsid_field(c.dsed));
    std::cout << "  DSED \"" << c.dsed << "\" -> ok=" << id.ok << " " << id.edition_number << "."
              << id.update_number << "\n";
    CCHECK(id.ok && id.edition_number == c.ed && id.update_number == c.upd);
    CCHECK(id.profile == "2");
  }
  for (const char* bad : {"", "9.x", "x", "9.", ".1"}) {
    const auto id = s100::parse_s101_dsid(dsid_field(bad));
    std::cout << "  DSED \"" << bad << "\" -> ok=" << id.ok << "\n";
    CCHECK(!id.ok);
  }
}

/** Reject with exactly `want`, dataset bit-for-bit unchanged. */
void expect_reject(const s100::Dataset& start, const fs::path& upd, UpdateRejectCode want,
                   const char* must_contain = nullptr) {
  CCHECK(fs::exists(upd));
  s100::Dataset ds = start;
  const auto before = dataset_digest(ds);
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  UpdateRejectCode code = UpdateRejectCode::None;
  const bool ok = s100::apply_s101_update(ds, upd, &err, &code);
  std::cout << "  " << upd.parent_path().filename().string() << "/" << upd.filename().string()
            << " -> " << name(code) << " | " << err.message << "\n";
  CCHECK(!ok);
  if (code != want) {
    std::cerr << "FAIL " << upd << " code " << name(code) << " want " << name(want) << "\n";
    std::exit(1);
  }
  CCHECK(dataset_digest(ds) == before);
  CCHECK(ds.cancelled == start.cancelled);
  if (must_contain) CCHECK(err.message.find(must_contain) != std::string::npos);
  ++g_seen[want];
}

s100::Dataset chain(const fs::path& b, const std::vector<fs::path>& ups) {
  s100::Dataset out;
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  s100::UpdateReport rep;
  CCHECK(s100::open_dataset_with_updates(b, ups, out, &err, &rep));
  return out;
}

void run() {
  check_fixture_set();
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};

  // ---- Oracle 0: positive controls first ----
  std::cout << "oracle 0 (DSED prerequisite):\n";
  dsed_unit_asserts();
  s100::Dataset nodot;
  CCHECK(s100::load_s101(cell("nodot", "000"), nodot, &err));
  CCHECK(nodot.edition_number == 9 && nodot.update_number == 0);
  {
    UpdateRejectCode code = UpdateRejectCode::OutOfSequence;
    if (!s100::apply_s101_update(nodot, cell("nodot", "001"), &err, &code) ||
        code != UpdateRejectCode::None || nodot.update_number != 1) {
      std::cerr << "FAIL nodot positive control: " << name(code) << " " << err.message << "\n";
      std::exit(1);
    }
  }
  s100::Dataset base;
  CCHECK(s100::load_s101(base_path(), base, &err));
  CCHECK(base.edition_number == 9 && base.update_number == 0);  // ED.UPD still parses
  const auto base_digest = dataset_digest(base);
  std::cout << "  nodot loads 9.0 and its 001 applies -> UPD " << nodot.update_number << "\n";
  for (const char* f : {"empty.001", "ed_x.001", "x.001", "ed_dot.001", "dot_upd.001"})
    expect_reject(base, upd_dir() / "dsed_bad" / f, UpdateRejectCode::BadStructure);

  s100::Dataset upd1 = base;
  CCHECK(s100::apply_s101_update(upd1, seq("001"), &err));
  CCHECK(upd1.dataset_name == "101AA00DS0003.000");  // D5: update never overwrites DSNM
  CCHECK(dataset_digest(nodot) == dataset_digest(upd1));

  // ---- Oracle 2: cancel ----
  std::cout << "oracle 2 (cancel):\n";
  {
    s100::Dataset ds = upd1;
    UpdateRejectCode code = UpdateRejectCode::OutOfSequence;
    CCHECK(s100::apply_s101_update(ds, cell("cancel", "002"), &err, &code));
    CCHECK(code == UpdateRejectCode::None);
    CCHECK(ds.cancelled);
    CCHECK(ds.features.empty() && ds.information.empty() && ds.spatial.records.empty());
    CCHECK(ds.edition_number == 9 && ds.update_number == 1);  // ED/UPD unchanged
    std::cout << "  cancel/002 -> applied, cancelled=1 features=0 info=0 spatial=0 ED.UPD 9.1\n";
    expect_reject(ds, cell("cancel", "003"), UpdateRejectCode::DatasetCancelled);
    // 003 is "valid-looking": it applies on the same chain without the cancel.
    s100::Dataset not_cancelled = upd1;
    CCHECK(s100::apply_s101_update(not_cancelled, cell("cancel", "003"), &err));

    s100::Dataset out;
    s100::UpdateReport rep;
    CCHECK(!s100::open_dataset_with_updates(
        base_path(), {cell("cancel", "001"), cell("cancel", "002"), cell("cancel", "003")}, out,
        &err, &rep));
    std::cout << "  open cancel/{001,002,003} -> applied=" << rep.applied << " cancelled="
              << rep.cancelled << " code=" << name(rep.code) << " notice=\"" << rep.notice
              << "\"\n";
    CCHECK(rep.code == UpdateRejectCode::DatasetCancelled);
    CCHECK(rep.applied == 2 && rep.cancelled);
    CCHECK(rep.rejected == cell("cancel", "003"));
    CCHECK(rep.notice.find(kCell) != std::string::npos);
    CCHECK(out.cancelled && out.features.empty());
    CCHECK(dataset_digest(out) == dataset_digest(ds));

    s100::UpdateReport rep2;
    CCHECK(s100::open_dataset_with_updates(base_path(), {cell("cancel", "001"), cell("cancel", "002")},
                                           out, &err, &rep2));
    CCHECK(rep2.cancelled && rep2.code == UpdateRejectCode::None);
    CCHECK(rep2.notice.find("cancelled: ") == 0 && rep2.notice.find(kCell) != std::string::npos);
  }

  // ---- Oracle 3: cancel_bad / wrong names / out-of-order cancel ----
  std::cout << "oracle 3 (bad cancel, D5 names, D4 out-of-order cancel):\n";
  expect_reject(upd1, cell("cancel_bad", "002"), UpdateRejectCode::BadStructure);
  expect_reject(base, upd_dir() / "cancel_wrong_name" / "101AA00DS0006.001",
                UpdateRejectCode::DatasetNameMismatch, "101AA00DS0006");
  expect_reject(upd1, upd_dir() / "cancel_wrong_name" / "101AA00DS0006.001",
                UpdateRejectCode::DatasetNameMismatch);
  expect_reject(base, upd_dir() / "wrong_name" / "101AA00DS0006.001",
                UpdateRejectCode::DatasetNameMismatch, "101AA00DS0006");
  {
    s100::Dataset out;
    s100::UpdateReport rep;
    CCHECK(s100::open_dataset_with_updates(
        base_path(), {cell("cancel_ooo", "001"), cell("cancel_ooo", "003")}, out, &err, &rep));
    std::cout << "  cancel_ooo {001, cancel 003} -> cancelled=" << rep.cancelled
              << " features=" << out.features.size() << " UPD=" << out.update_number << "\n";
    CCHECK(rep.cancelled && out.cancelled && out.features.empty());
    s100::Dataset direct = base;  // no update at all before the cancel: still executes (D4)
    CCHECK(s100::apply_s101_update(direct, cell("cancel_ooo", "003"), &err));
    CCHECK(direct.cancelled);
  }

  // ---- Oracle 4: re-issue ----
  std::cout << "oracle 4 (re-issue):\n";
  {
    s100::Dataset out;
    s100::UpdateReport rep;
    CCHECK(s100::open_dataset_with_updates(
        cell("reissue", "000"),
        {cell("reissue", "001"), cell("reissue", "002"), cell("reissue", "003")}, out, &err, &rep));
    const auto ref = chain(base_path(), {seq("001"), seq("002"), seq("003")});
    const auto d_out = dataset_digest(out), d_ref = dataset_digest(ref);
    std::cout << "  reissue 9.2 + {001,002,003} -> skipped=" << rep.skipped << " applied="
              << rep.applied << " ED.UPD " << out.edition_number << "." << out.update_number
              << " digest " << d_out.substr(0, 16) << " vs base+001+002+003 " << d_ref.substr(0, 16)
              << "\n";
    CCHECK(rep.skipped == 2 && rep.applied == 1 && rep.code == UpdateRejectCode::None);
    CCHECK(out.edition_number == 9 && out.update_number == 3);
    CCHECK(ref.edition_number == 9 && ref.update_number == 3);
    CCHECK(by_rcid(out, kDeletedRcid) == nullptr);  // 003's change visible
    CCHECK(by_rcid(out, kInsertedRcid) != nullptr);
    CCHECK(d_out == d_ref);
    // Outside the window duplicates stay OutOfSequence.
    s100::Dataset dup;
    s100::UpdateReport rep_dup;
    CCHECK(!s100::open_dataset_with_updates(cell("reissue", "000"),
                                            {cell("reissue", "003"), cell("reissue", "003")}, dup,
                                            &err, &rep_dup));
    CCHECK(rep_dup.code == UpdateRejectCode::OutOfSequence && rep_dup.applied == 1);
  }

  // ---- Oracle 5: new edition ----
  std::cout << "oracle 5 (new edition):\n";
  {
    s100::ExchangeCatalogue cat;
    CCHECK(s100::parse_exchange_catalogue(upd_dir() / "newed" / "CATALOG.XML", cat, &err));
    const auto series = s100::group_cell_series(cat);
    CCHECK(series.size() == 1);
    s100::Dataset out;
    s100::UpdateReport rep;
    CCHECK(s100::open_cell_series(series[0], out, &err, &rep));
    s100::Dataset standalone;
    CCHECK(s100::open_dataset(cell("newed", "000"), standalone, &err));
    std::cout << "  newed -> ED.UPD " << out.edition_number << "." << out.update_number
              << " applied=" << rep.applied << " superseded=" << rep.superseded.size()
              << " notice=\"" << rep.notice << "\"\n";
    CCHECK(out.edition_number == 10 && out.update_number == 0);
    CCHECK(rep.applied == 0);
    CCHECK(rep.superseded.size() == 1);
    CCHECK(fs::equivalent(rep.superseded[0], cell("newed", "001")));
    CCHECK(rep.notice.find("superseded updates can be removed") != std::string::npos);
    CCHECK(dataset_digest(out) == dataset_digest(standalone));
    expect_reject(out, cell("newed", "001"), UpdateRejectCode::EditionMismatch);
  }

  // ---- Oracle 6: newer edition pending ----
  std::cout << "oracle 6 (newer edition pending):\n";
  expect_reject(base, cell("pending", "001"), UpdateRejectCode::NewerEditionPending, "edition 10");
  {
    s100::Dataset out;
    s100::UpdateReport rep;
    CCHECK(!s100::open_dataset_with_updates(base_path(), {cell("pending", "001")}, out, &err, &rep));
    CCHECK(rep.code == UpdateRejectCode::NewerEditionPending);
    CCHECK(rep.notice.find("newer edition 10 pending") != std::string::npos);
    CCHECK(dataset_digest(out) == base_digest);
  }

  // ---- Oracle 7: new codes covered and named ----
  CCHECK(std::string(name(UpdateRejectCode::DatasetCancelled)) == "DatasetCancelled");
  CCHECK(std::string(name(UpdateRejectCode::NewerEditionPending)) == "NewerEditionPending");
  CCHECK(std::string(name(UpdateRejectCode::DatasetNameMismatch)) == "DatasetNameMismatch");
  for (auto c : {UpdateRejectCode::DatasetCancelled, UpdateRejectCode::NewerEditionPending,
                 UpdateRejectCode::DatasetNameMismatch})
    if (g_seen[c] == 0) {
      std::cerr << "FAIL no fixture exercised " << name(c) << "\n";
      std::exit(1);
    }

  // ---- Oracles 8 / 9: session ----
  const auto key = s100::exchange_member_key(s100::ProductId::S101, kCell);
  const fs::path sr = upd_dir() / "session_replace";
  {
    std::cout << "oracle 8 (session re-issue replaces):\n";
    s100::ChartSession session;
    CCHECK(session.open_exchange_set(sr / "A" / "CATALOG.XML", &err));
    CCHECK(session.load_member(key, &err));
    CCHECK(session.find(key) && session.find(key)->update_number == 2);
    const auto d_a = dataset_digest(*session.find(key));
    CCHECK(session.open_exchange_set(sr / "B" / "CATALOG.XML", &err));
    CCHECK(session.load_member(key, &err));
    const s100::Dataset* m = session.find(key);
    CCHECK(m != nullptr);
    s100::ExchangeCatalogue cat;
    CCHECK(s100::parse_exchange_catalogue(sr / "B" / "CATALOG.XML", cat, &err));
    s100::Dataset fresh;
    CCHECK(s100::open_cell_series(s100::group_cell_series(cat).front(), fresh, &err));
    std::cout << "  A -> UPD 2; B -> UPD " << m->update_number << " skipped="
              << session.last_update_report(key).skipped << " digest==fresh B: "
              << (dataset_digest(*m) == dataset_digest(fresh)) << "\n";
    CCHECK(m->update_number == 3);
    CCHECK(dataset_digest(*m) == dataset_digest(fresh));
    CCHECK(dataset_digest(*m) != d_a);
    CCHECK(dataset_digest(*m) == dataset_digest(chain(base_path(), {seq("001"), seq("002"), seq("003")})));
  }
  {
    std::cout << "oracle 9 (session cancel, then fresh edition):\n";
    s100::ChartSession session;
    CCHECK(session.open_exchange_set(sr / "X" / "CATALOG.XML", &err));
    CCHECK(session.series().front().updates.size() == 2);
    session.load_member(key, &err);
    const s100::Dataset* m = session.find(key);
    const auto rep = session.last_update_report(key);
    std::cout << "  X -> find=" << (m ? "member" : "null") << " cancelled="
              << (m ? m->cancelled : false) << " features=" << (m ? m->features.size() : 0)
              << " report.cancelled=" << rep.cancelled << " notice=\"" << rep.notice << "\"\n";
    CCHECK(m != nullptr && m->cancelled && m->features.empty());
    CCHECK(rep.cancelled);
    CCHECK(session.open_exchange_set(sr / "C" / "CATALOG.XML", &err));
    CCHECK(session.load_member(key, &err));
    m = session.find(key);
    std::cout << "  C -> cancelled=" << m->cancelled << " ED.UPD " << m->edition_number << "."
              << m->update_number << " features=" << m->features.size() << "\n";
    CCHECK(m && !m->cancelled && m->edition_number == 10 && !m->features.empty());
    CCHECK(!session.last_update_report(key).cancelled);
  }
  std::cout << "exchange_cancel_reissue ok\n";
}

}  // namespace

void test_exchange_cancel_reissue() {
  try {
    run();
  } catch (const std::exception& e) {  // oracle 10: nothing escapes
    std::cerr << "FAIL exception: " << e.what() << "\n";
    std::exit(1);
  }
}
