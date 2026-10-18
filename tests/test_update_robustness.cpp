// IHO-S164-UPD-SEQ / IHO-S164-UPD-CORRUPT — docs/update-robustness-contract.md rev 1.2.
// CTest: exchange_update_robustness. MUST NOT SKIP: a missing fixture is a FAIL.
#include "s100/exchange_set.hpp"
#include "support/dataset_digest.hpp"
#include "support/sha256.hpp"

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

#define UCHECK(cond)                                                                \
  do {                                                                              \
    if (!(cond)) {                                                                  \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";  \
      std::exit(1);                                                                 \
    }                                                                               \
  } while (0)

const char* name(UpdateRejectCode c) { return s100::update_reject_code_name(c); }

// Values chosen by tools/gen_update_fixtures.py (seed 20261018); printed as "meta ..." by it.
constexpr std::uint32_t kInsertedRcid = 81;   // seq/.001 inserts DepthContour FRID 81
constexpr const char* kPatchedValue = "12";   // seq/.002 sets its valueOfDepthContour
constexpr std::uint32_t kDeletedRcid = 35;    // seq/.003 deletes original FRID 35
constexpr std::uint32_t kLateModify = 32;     // late_fail: modify FRID 32 valueOfDepthContour=99
constexpr std::uint32_t kLateDelete = 33;     // late_fail: delete FRID 33

fs::path s101_dir() { return fs::path(S100_SOURCE_DIR) / "testdata" / "s101"; }
fs::path upd_dir() { return s101_dir() / "updates"; }
fs::path base_path() { return s101_dir() / "101AA00DS0003.000"; }
fs::path seq(int n) {
  char ext[8];
  std::snprintf(ext, sizeof ext, ".%03d", n);
  return upd_dir() / "seq" / (std::string("101AA00DS0003") + ext);
}

const s100::Feature* by_rcid(const s100::Dataset& ds, std::uint32_t rcid) {
  for (const auto& f : ds.features)
    if (f.record_id == rcid) return &f;
  return nullptr;
}

/** fixtures.toml [[update_fixture]] path -> sha256 (minimal line parser; tolerates CRLF). */
std::map<std::string, std::string> manifest() {
  std::map<std::string, std::string> m;
  std::ifstream in(s101_dir() / "fixtures.toml", std::ios::binary);
  UCHECK(in.good());
  std::string line, path;
  bool in_fixture = false;
  auto value = [](const std::string& l) {
    const auto a = l.find('"'), b = l.rfind('"');
    return a == std::string::npos || b <= a ? std::string() : l.substr(a + 1, b - a - 1);
  };
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.rfind("[", 0) == 0) {
      in_fixture = line == "[[update_fixture]]";
      path.clear();
      continue;
    }
    if (!in_fixture) continue;
    if (line.rfind("path = ", 0) == 0) path = value(line);
    if (line.rfind("sha256 = ", 0) == 0 && !path.empty()) m[path] = value(line);
  }
  return m;
}

/** Oracle #1 (existence + sha256). Missing fixture -> FAIL, never SKIP. */
void check_fixture_set() {
  const std::vector<std::string> expected = {
      "updates/seq/101AA00DS0003.001",   "updates/seq/101AA00DS0003.002",
      "updates/seq/101AA00DS0003.003",   "updates/bad_ed/101AA00DS0003.001",
      "updates/notupd/101AA00DS0003.000", "updates/corrupt/trunc.001",
      "updates/corrupt/leader.001",      "updates/corrupt/ruin9.001",
      "updates/corrupt/missing.001",     "updates/corrupt/late_fail.001",
      "updates/catalog/CATALOG.XML"};
  const auto m = manifest();
  UCHECK(m.size() == expected.size());
  for (const auto& rel : expected) {
    const fs::path p = s101_dir() / fs::path(rel);
    if (!fs::exists(p)) {
      std::cerr << "FAIL missing fixture " << p << " (run tools/gen_update_fixtures.py)\n";
      std::exit(1);
    }
    auto it = m.find(rel);
    UCHECK(it != m.end());
    const auto got = s100test::sha256_file_hex(p);
    if (got != it->second) {
      std::cerr << "FAIL sha256 " << rel << " got " << got << " want " << it->second << "\n";
      std::exit(1);
    }
  }
  std::cout << "fixtures ok: " << expected.size() << " files, sha256 match fixtures.toml\n";
}

std::map<UpdateRejectCode, int> g_seen;

/** Oracle #2/#4/#6 for one apply_s101_update reject. */
void expect_apply_reject(const s100::Dataset& start, const fs::path& upd, UpdateRejectCode want,
                         const std::string& want_edupd) {
  UCHECK(fs::exists(upd));
  s100::Dataset ds = start;
  const auto before = s100test::dataset_digest(ds);
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  UpdateRejectCode code = UpdateRejectCode::None;
  bool ok = true;
  try {
    ok = s100::apply_s101_update(ds, upd, &err, &code);
  } catch (const std::exception& e) {
    std::cerr << "FAIL exception escaped for " << upd << ": " << e.what() << "\n";
    std::exit(1);
  }
  const auto after = s100test::dataset_digest(ds);
  std::cout << "  " << upd.filename().string() << " -> " << name(code) << " | " << err.message
            << "\n";
  UCHECK(!ok);
  if (code != want) {
    std::cerr << "FAIL " << upd << " code " << name(code) << " want " << name(want) << "\n";
    std::exit(1);
  }
  UCHECK(before == after);  // rollback (rule 1, oracle #4)
  UCHECK(err.message.find(upd.filename().string()) != std::string::npos);
  UCHECK(err.message.find(want_edupd) != std::string::npos);
  ++g_seen[want];
}

/** Oracle #2/#4/#6 for a sequence reject through open_dataset_with_updates. */
void expect_sequence_reject(const char* label, const std::vector<fs::path>& updates,
                            std::size_t reject_index, int want_applied,
                            const std::string& want_edupd, const s100::Dataset& prefix_state) {
  s100::Dataset out;
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  s100::UpdateReport rep;
  const bool ok = s100::open_dataset_with_updates(base_path(), updates, out, &err, &rep);
  std::cout << "  " << label << " -> applied=" << rep.applied << " rejected="
            << rep.rejected.filename().string() << " code=" << name(rep.code) << " | "
            << rep.message << "\n";
  UCHECK(!ok);
  UCHECK(rep.code == UpdateRejectCode::OutOfSequence);
  UCHECK(rep.applied == want_applied);
  UCHECK(rep.rejected == updates[reject_index]);
  UCHECK(rep.message.find(updates[reject_index].filename().string()) != std::string::npos);
  UCHECK(rep.message.find(want_edupd) != std::string::npos);
  // out = base + applied prefix, bit-for-bit (rule 2 + oracle #4).
  UCHECK(s100test::dataset_digest(out) == s100test::dataset_digest(prefix_state));
  UCHECK(out.update_number == want_applied);
  ++g_seen[UpdateRejectCode::OutOfSequence];
}

}  // namespace

void test_exchange_update_robustness() {
  check_fixture_set();

  s100::Dataset base;
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  UCHECK(s100::load_s101(base_path(), base, &err));
  UCHECK(base.edition_number == 9 && base.update_number == 0);
  const std::size_t base_count = base.features.size();
  const auto base_digest = s100test::dataset_digest(base);
  UCHECK(by_rcid(base, kInsertedRcid) == nullptr);
  UCHECK(by_rcid(base, kDeletedRcid) != nullptr);

  // Oracle #0: positive control first — FAIL before any reject case if it does not apply.
  s100::Dataset upd1 = base;
  {
    UpdateRejectCode code = UpdateRejectCode::OutOfSequence;  // must be reset to None
    s100::LoadError e{s100::LoadErrorKind::Invalid, {}};
    const bool ok = s100::apply_s101_update(upd1, seq(1), &e, &code);
    if (!ok || code != UpdateRejectCode::None || upd1.update_number != 1) {
      std::cerr << "FAIL positive control " << seq(1) << " ok=" << ok << " code=" << name(code)
                << " upd=" << upd1.update_number << " msg=" << e.message << "\n";
      std::exit(1);
    }
    std::cout << "positive control ok: " << seq(1).filename().string() << " -> UPD "
              << upd1.update_number << "\n";
  }
  UCHECK(s100test::dataset_digest(base) == base_digest);  // copy untouched

  // Oracle #3: valid chain.
  const auto* ins = by_rcid(upd1, kInsertedRcid);
  UCHECK(ins != nullptr);
  UCHECK(ins->feature_type == "DepthContour");
  UCHECK(upd1.features.size() == base_count + 1);
  s100::Dataset upd2 = upd1;
  UCHECK(s100::apply_s101_update(upd2, seq(2), &err));
  UCHECK(upd2.update_number == 2);
  UCHECK(by_rcid(upd2, kInsertedRcid) != nullptr);
  UCHECK(by_rcid(upd2, kInsertedRcid)->attribute("valueOfDepthContour") != nullptr);
  UCHECK(*by_rcid(upd2, kInsertedRcid)->attribute("valueOfDepthContour") == kPatchedValue);
  s100::Dataset upd3 = upd2;
  UCHECK(s100::apply_s101_update(upd3, seq(3), &err));
  UCHECK(upd3.update_number == 3);
  UCHECK(by_rcid(upd3, kDeletedRcid) == nullptr);
  UCHECK(upd3.features.size() == base_count + 1 - 1);
  {
    s100::Dataset chain;
    s100::UpdateReport rep;
    UCHECK(s100::open_dataset_with_updates(base_path(), {seq(1), seq(2), seq(3)}, chain, &err, &rep));
    UCHECK(rep.applied == 3 && rep.code == UpdateRejectCode::None && rep.rejected.empty());
    UCHECK(chain.update_number == 3);
    UCHECK(s100test::dataset_digest(chain) == s100test::dataset_digest(upd3));
    std::cout << "valid chain ok: applied=3 UPD=3 features " << base_count << " -> "
              << chain.features.size() << "\n";
  }

  std::cout << "reject cases:\n";
  // Sequence rows (oracle #2): reversed, duplicate, gap.
  expect_sequence_reject("reversed 002,001", {seq(2), seq(1)}, 0, 0, "9.2", base);
  expect_sequence_reject("duplicate 001,001", {seq(1), seq(1)}, 1, 1, "9.1", upd1);
  expect_sequence_reject("gap 001,003", {seq(1), seq(3)}, 1, 1, "9.3", upd1);
  // Same rows at the apply_s101_update level (digest unchanged).
  expect_apply_reject(base, seq(2), UpdateRejectCode::OutOfSequence, "9.2");   // gap n+2
  expect_apply_reject(upd1, seq(1), UpdateRejectCode::OutOfSequence, "9.1");   // duplicate n
  expect_apply_reject(base, upd_dir() / "bad_ed" / "101AA00DS0003.001",
                      UpdateRejectCode::EditionMismatch, "8.1");
  expect_apply_reject(base, upd_dir() / "notupd" / "101AA00DS0003.000",
                      UpdateRejectCode::NotUpdateFile, "9.0");
  // trunc.001 is cut inside the DDR, so its DSID is not on disk: message says it is unreadable.
  expect_apply_reject(base, upd_dir() / "corrupt" / "trunc.001", UpdateRejectCode::Truncated,
                      "ED.UPD ?.?");
  expect_apply_reject(base, upd_dir() / "corrupt" / "leader.001", UpdateRejectCode::BadStructure,
                      "9.1");
  expect_apply_reject(base, upd_dir() / "corrupt" / "ruin9.001", UpdateRejectCode::BadInstruction,
                      "9.1");
  expect_apply_reject(base, upd_dir() / "corrupt" / "missing.001", UpdateRejectCode::MissingTarget,
                      "9.1");
  expect_apply_reject(base, upd_dir() / "corrupt" / "late_fail.001",
                      UpdateRejectCode::MissingTarget, "9.1");

  // Oracle #5: none of late_fail's 3 valid ops is visible.
  {
    s100::Dataset ds = base;
    UpdateRejectCode code = UpdateRejectCode::None;
    UCHECK(!s100::apply_s101_update(ds, upd_dir() / "corrupt" / "late_fail.001", &err, &code));
    UCHECK(by_rcid(ds, kInsertedRcid) == nullptr);
    UCHECK(by_rcid(ds, kLateDelete) != nullptr);
    UCHECK(by_rcid(ds, kLateModify) != nullptr);
    UCHECK(*by_rcid(ds, kLateModify)->attribute("valueOfDepthContour") ==
           *by_rcid(base, kLateModify)->attribute("valueOfDepthContour"));
    UCHECK(*by_rcid(ds, kLateModify)->attribute("valueOfDepthContour") != "99");
    UCHECK(ds.features.size() == base_count);
  }

  // CatalogueMismatch through the exchange-set path (open_cell_series).
  {
    const fs::path cat_path = upd_dir() / "catalog" / "CATALOG.XML";
    s100::ExchangeCatalogue cat;
    UCHECK(s100::parse_exchange_catalogue(cat_path, cat, &err));
    const auto series = s100::group_cell_series(cat);
    UCHECK(series.size() == 1);
    UCHECK(series[0].updates.size() == 1);
    UCHECK(fs::equivalent(series[0].updates[0], seq(1)));
    s100::Dataset out;
    s100::UpdateReport rep;
    const bool ok = s100::open_cell_series(series[0], out, &err, &rep);
    std::cout << "  catalog/CATALOG.XML -> applied=" << rep.applied << " code=" << name(rep.code)
              << " | " << rep.message << "\n";
    UCHECK(!ok);
    UCHECK(rep.code == UpdateRejectCode::CatalogueMismatch);
    UCHECK(rep.applied == 0);
    UCHECK(rep.message.find("101AA00DS0003.001") != std::string::npos);
    UCHECK(rep.message.find("9.1") != std::string::npos);
    UCHECK(s100test::dataset_digest(out) == base_digest);  // last good state = base
    ++g_seen[UpdateRejectCode::CatalogueMismatch];
  }

  // Every enum value except None has >= 1 fixture.
  for (auto c : {UpdateRejectCode::EditionMismatch, UpdateRejectCode::OutOfSequence,
                 UpdateRejectCode::NotUpdateFile, UpdateRejectCode::Truncated,
                 UpdateRejectCode::BadStructure, UpdateRejectCode::BadInstruction,
                 UpdateRejectCode::MissingTarget, UpdateRejectCode::CatalogueMismatch}) {
    if (g_seen[c] == 0) {
      std::cerr << "FAIL no fixture exercised " << name(c) << "\n";
      std::exit(1);
    }
  }
  UCHECK(s100test::dataset_digest(base) == base_digest);
  std::cout << "exchange_update_robustness ok\n";
}
