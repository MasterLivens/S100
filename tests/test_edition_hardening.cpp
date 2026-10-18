// IHO-S164-EDITION-HARDEN — docs/edition-hardening-contract.md rev 1.
// CTest: exchange_edition_hardening. MUST NOT SKIP: a missing fixture is a FAIL.
#include "s100/dataset.hpp"
#include "s100/exchange_set.hpp"
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
#include <set>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

#ifndef S100_SOURCE_DIR
#error "S100_SOURCE_DIR required"
#endif
#ifndef S100_CLI_PATH
#error "S100_CLI_PATH required"
#endif

namespace {

namespace fs = std::filesystem;
using s100::UpdateEditionRelation;
using s100::UpdateRejectCode;
using s100test::dataset_digest;

#define ECHECK(cond)                                                                \
  do {                                                                              \
    if (!(cond)) {                                                                  \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";  \
      std::exit(1);                                                                 \
    }                                                                               \
  } while (0)

const char* name(UpdateRejectCode c) { return s100::update_reject_code_name(c); }
const char* kCell = "101AA00DS0003";

fs::path s101_dir() { return fs::path(S100_SOURCE_DIR) / "testdata" / "s101"; }
fs::path upd_dir() { return s101_dir() / "updates"; }
fs::path base_path() { return s101_dir() / "101AA00DS0003.000"; }
fs::path cell(const std::string& dir, const char* ext) {
  return upd_dir() / fs::path(dir) / (std::string(kCell) + "." + ext);
}
fs::path ed_cell(const std::string& c, const char* ext) { return cell("edition/" + c, ext); }

std::set<std::string> canon(const std::vector<fs::path>& v) {
  std::set<std::string> s;
  for (const auto& p : v) s.insert(fs::weakly_canonical(p).generic_string());
  return s;
}
std::set<std::string> canon_files(std::initializer_list<fs::path> v) {
  return canon(std::vector<fs::path>(v));
}

/** fixtures.toml [[edition_fixture]] path -> sha256. */
std::map<std::string, std::string> edition_manifest() {
  std::map<std::string, std::string> m;
  std::ifstream in(s101_dir() / "fixtures.toml", std::ios::binary);
  ECHECK(in.good());
  std::string line, path;
  bool in_fixture = false;
  auto value = [](const std::string& l) {
    const auto a = l.find('"'), b = l.rfind('"');
    return a == std::string::npos || b <= a ? std::string() : l.substr(a + 1, b - a - 1);
  };
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.rfind("[", 0) == 0) {
      in_fixture = line == "[[edition_fixture]]";
      path.clear();
      continue;
    }
    if (!in_fixture) continue;
    if (line.rfind("path = ", 0) == 0) path = value(line);
    if (line.rfind("sha256 = ", 0) == 0 && !path.empty()) m[path] = value(line);
  }
  return m;
}

const std::vector<std::string> kBadBases = {"base_dsed_empty", "base_dsed_ed_x", "base_dsed_x",
                                            "base_dsed_ed_dot", "base_dsed_dot_upd",
                                            "base_dsed_zero", "base_no_dsid"};

/** Oracle 1: every [[edition_fixture]] exists and matches its sha256. */
void check_fixture_set() {
  std::vector<std::string> expected;
  for (const auto& b : kBadBases) expected.push_back("updates/edition/" + b + "/101AA00DS0003.000");
  for (const char* rel :
       {"updates/edition/base_bad_cat/CATALOG.XML", "updates/edition/old_left/101AA00DS0003.000",
        "updates/edition/old_left/101AA00DS0003.001", "updates/edition/old_left/101AA00DS0003.002",
        "updates/edition/mixed/101AA00DS0003.000", "updates/edition/mixed/101AA00DS0003.001",
        "updates/edition/mixed/101AA00DS0003.002", "updates/edition/nocat/CATALOG.XML"})
    expected.push_back(rel);
  const auto m = edition_manifest();
  if (m.empty()) {
    std::cerr << "FAIL fixtures.toml has no [[edition_fixture]] table\n";
    std::exit(1);
  }
  ECHECK(m.size() == expected.size());
  for (const auto& rel : expected) {
    const fs::path p = s101_dir() / fs::path(rel);
    if (!fs::exists(p)) {
      std::cerr << "FAIL missing fixture " << p << " (run tools/gen_update_fixtures.py)\n";
      std::exit(1);
    }
    auto it = m.find(rel);
    ECHECK(it != m.end());
    if (s100test::sha256_file_hex(p) != it->second) {
      std::cerr << "FAIL sha256 " << rel << "\n";
      std::exit(1);
    }
  }
  std::cout << "oracle 1: " << expected.size() << " edition fixtures, sha256 match fixtures.toml\n";
}

s100::Dataset load_ok(const fs::path& p) {
  s100::Dataset ds;
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  if (!s100::open_dataset(p, ds, &err)) {
    std::cerr << "FAIL open_dataset " << p << ": " << err.message << "\n";
    std::exit(1);
  }
  return ds;
}

struct SeriesResult {
  bool ok{false};
  s100::Dataset ds;
  s100::UpdateReport rep;
  std::string digest;
};

SeriesResult with_updates(const fs::path& b, const std::vector<fs::path>& ups) {
  SeriesResult r;
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  r.ok = s100::open_dataset_with_updates(b, ups, r.ds, &err, &r.rep);
  r.digest = dataset_digest(r.ds);
  return r;
}

SeriesResult via_series(const fs::path& catalog) {
  s100::ExchangeCatalogue cat;
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  ECHECK(s100::parse_exchange_catalogue(catalog, cat, &err));
  const auto series = s100::group_cell_series(cat);
  ECHECK(series.size() == 1);
  SeriesResult r;
  r.ok = s100::open_cell_series(series.front(), r.ds, &err, &r.rep);
  r.digest = dataset_digest(r.ds);
  return r;
}

void print(const char* label, const SeriesResult& r) {
  std::cout << "  " << label << " -> ok=" << r.ok << " code=" << name(r.rep.code)
            << " applied=" << r.rep.applied << " superseded=" << r.rep.superseded.size()
            << " ED.UPD " << r.ds.edition_number << "." << r.ds.update_number << " notice=\""
            << r.rep.notice << "\"\n";
}

void expect_superseded_only(const char* label, const SeriesResult& r,
                            const std::set<std::string>& want, const std::string& want_digest) {
  print(label, r);
  ECHECK(r.ok);
  ECHECK(r.rep.code == UpdateRejectCode::None);
  ECHECK(r.rep.rejected.empty());
  ECHECK(r.rep.applied == 0);
  ECHECK(canon(r.rep.superseded) == want);
  ECHECK(r.rep.notice.find("superseded updates can be removed") != std::string::npos);
  ECHECK(r.digest == want_digest);
}

/** Standalone apply: exactly `want`, digest unchanged. */
void expect_standalone(const s100::Dataset& start, const fs::path& upd, UpdateRejectCode want) {
  ECHECK(fs::exists(upd));
  s100::Dataset ds = start;
  const auto before = dataset_digest(ds);
  s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
  UpdateRejectCode code = UpdateRejectCode::None;
  const bool ok = s100::apply_s101_update(ds, upd, &err, &code);
  std::cout << "  standalone " << upd.parent_path().filename().string() << "/"
            << upd.filename().string() << " on ED " << start.edition_number << " -> "
            << name(code) << "\n";
  ECHECK(!ok);
  ECHECK(code == want);
  ECHECK(dataset_digest(ds) == before);
}

struct CliResult {
  int exit_code{0};
  std::string out, err;
};

CliResult run_cli(const std::string& args) {
  const auto tag = std::to_string(std::rand());
  const fs::path out_path = fs::temp_directory_path() / ("s100_edition_out_" + tag + ".txt");
  const fs::path err_path = fs::temp_directory_path() / ("s100_edition_err_" + tag + ".txt");
#if defined(_WIN32)
  const std::string cmd = std::string("cmd /S /C \"\"") + S100_CLI_PATH + "\" " + args +
                          " 1>\"" + out_path.string() + "\" 2>\"" + err_path.string() + "\"\"";
#else
  const std::string cmd = std::string("\"") + S100_CLI_PATH + "\" " + args + " >\"" +
                          out_path.string() + "\" 2>\"" + err_path.string() + "\"";
#endif
  CliResult r;
  r.exit_code = std::system(cmd.c_str());
#if !defined(_WIN32)
  if (WIFEXITED(r.exit_code)) r.exit_code = WEXITSTATUS(r.exit_code);
#endif
  auto slurp = [](const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  };
  r.out = slurp(out_path);
  r.err = slurp(err_path);
  std::error_code ec;
  fs::remove(out_path, ec);
  fs::remove(err_path, ec);
  return r;
}

std::string q(const fs::path& p) { return "\"" + p.string() + "\""; }

void run() {
  // ---- Oracle 0: positive controls first ----
  std::cout << "oracle 0 (positive controls):\n";
  const s100::Dataset root = load_ok(base_path());
  ECHECK(root.edition_number == 9 && root.update_number == 0);
  const s100::Dataset nodot = load_ok(cell("nodot", "000"));
  ECHECK(nodot.edition_number == 9 && nodot.update_number == 0);
  {
    const auto r = with_updates(base_path(), {cell("seq", "001"), cell("seq", "002"),
                                              cell("seq", "003")});
    print("seq 001-003", r);
    ECHECK(r.ok && r.rep.applied == 3 && r.rep.code == UpdateRejectCode::None);
  }
  struct Rel { int inst, upd; UpdateEditionRelation want; };
  for (const Rel& c : {Rel{9, 0, UpdateEditionRelation::Cancel},
                       Rel{9, 9, UpdateEditionRelation::Current},
                       Rel{9, 10, UpdateEditionRelation::NewerPending},
                       Rel{10, 9, UpdateEditionRelation::Superseded},
                       Rel{10, 1, UpdateEditionRelation::Superseded}}) {
    ECHECK(s100::classify_update_edition(c.inst, c.upd) == c.want);
  }
  std::cout << "  classify_update_edition table: 5/5\n";

  check_fixture_set();

  // ---- Oracle 2: viewer scenario (sibling discovery -> open_dataset_with_updates) ----
  std::cout << "oracle 2 (viewer scenario):\n";
  const fs::path newed = cell("newed", "000");
  const std::string newed_digest = dataset_digest(load_ok(newed));
  {
    const auto ups = s100::discover_update_files(newed);
    expect_superseded_only("newed", with_updates(newed, ups), canon_files({cell("newed", "001")}),
                           newed_digest);
    const fs::path ol = ed_cell("old_left", "000");
    const s100::Dataset ol_base = load_ok(ol);
    ECHECK(ol_base.edition_number == 10 && ol_base.update_number == 0);
    expect_superseded_only("old_left", with_updates(ol, s100::discover_update_files(ol)),
                           canon_files({ed_cell("old_left", "001"), ed_cell("old_left", "002")}),
                           dataset_digest(ol_base));
  }

  // ---- Oracle 3: mixed ----
  std::cout << "oracle 3 (mixed):\n";
  {
    const fs::path mb = ed_cell("mixed", "000");
    const auto r = with_updates(mb, s100::discover_update_files(mb));
    print("mixed", r);
    ECHECK(r.ok && r.rep.code == UpdateRejectCode::None && r.rep.applied == 1);
    ECHECK(r.ds.edition_number == 10 && r.ds.update_number == 1);
    ECHECK(canon(r.rep.superseded) == canon_files({ed_cell("mixed", "002")}));
    s100::Dataset alone = load_ok(mb);
    s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
    ECHECK(s100::apply_s101_update(alone, ed_cell("mixed", "001"), &err));
    ECHECK(r.digest == dataset_digest(alone));
    std::cout << "  digest == base + .001 alone: 1\n";
  }

  // ---- Oracle 4: parity open_cell_series vs open_dataset_with_updates; session nocat ----
  std::cout << "oracle 4 (parity):\n";
  {
    struct P { const char* label; fs::path catalog; fs::path base; };
    for (const P& p : {P{"newed", upd_dir() / "newed" / "CATALOG.XML", newed},
                       P{"nocat", upd_dir() / "edition" / "nocat" / "CATALOG.XML",
                         ed_cell("old_left", "000")}}) {
      const auto a = via_series(p.catalog);
      const auto b = with_updates(p.base, s100::discover_update_files(p.base));
      print((std::string(p.label) + " open_cell_series").c_str(), a);
      print((std::string(p.label) + " open_dataset_with_updates").c_str(), b);
      ECHECK(a.ok && b.ok);
      ECHECK(a.digest == b.digest);
      ECHECK(a.rep.applied == b.rep.applied);
      ECHECK(a.rep.code == b.rep.code);
      ECHECK(canon(a.rep.superseded) == canon(b.rep.superseded));
      ECHECK(!a.rep.superseded.empty());
    }
    s100::ChartSession session;
    s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
    ECHECK(session.open_exchange_set(upd_dir() / "edition" / "nocat" / "CATALOG.XML", &err));
    const auto key = s100::exchange_member_key(s100::ProductId::S101, kCell);
    ECHECK(session.load_member(key, &err));
    const s100::Dataset* m = session.find(key);
    const auto rep = session.last_update_report(key);
    std::cout << "  session nocat -> member ED.UPD " << (m ? m->edition_number : -1) << "."
              << (m ? m->update_number : -1) << " superseded=" << rep.superseded.size()
              << " code=" << name(rep.code) << "\n";
    ECHECK(m && m->edition_number == 10 && m->update_number == 0);
    ECHECK(rep.superseded.size() == 2 && rep.code == UpdateRejectCode::None);
  }

  // ---- Oracle 5: earlier codes win over superseded ----
  std::cout << "oracle 5 (earlier codes win):\n";
  {
    struct C { fs::path upd; UpdateRejectCode want; };
    for (const C& c : {C{upd_dir() / "wrong_name" / "101AA00DS0006.001",
                         UpdateRejectCode::DatasetNameMismatch},
                       C{upd_dir() / "dsed_bad" / "x.001", UpdateRejectCode::BadStructure}}) {
      ECHECK(fs::exists(c.upd));
      const auto r = with_updates(newed, {c.upd});
      print(c.upd.filename().string().c_str(), r);
      ECHECK(!r.ok && r.rep.code == c.want);
      ECHECK(r.rep.superseded.empty());
      ECHECK(r.digest == newed_digest);
    }
  }

  // ---- Oracle 6: standalone apply_s101_update keeps EditionMismatch (E2) ----
  std::cout << "oracle 6 (standalone):\n";
  {
    const s100::Dataset ed10 = load_ok(newed);
    expect_standalone(ed10, cell("newed", "001"), UpdateRejectCode::EditionMismatch);
    expect_standalone(root, cell("bad_ed", "001"), UpdateRejectCode::EditionMismatch);
    expect_standalone(root, cell("pending", "001"), UpdateRejectCode::NewerEditionPending);
  }

  // ---- Oracles 7 / 8: strict base DSED ----
  std::cout << "oracles 7/8 (base rejected):\n";
  const std::string root_digest = dataset_digest(root);
  for (const auto& b : kBadBases) {
    const fs::path p = ed_cell(b, "000");
    s100::Dataset out = root;  // pre-filled: must keep its digest
    s100::LoadError err{s100::LoadErrorKind::Io, {}};
    const bool ok = s100::open_dataset(p, out, &err);
    std::cout << "  " << b << " open_dataset -> ok=" << ok << " | " << err.message << "\n";
    ECHECK(!ok);
    ECHECK(err.kind == s100::LoadErrorKind::Invalid);
    ECHECK(err.message.find(p.string()) != std::string::npos);
    const bool want_no_dsid = b == "base_no_dsid";
    ECHECK(err.message.find(want_no_dsid ? "no DSID" : "DSED") != std::string::npos);
    ECHECK(dataset_digest(out) == root_digest);
    s100::Dataset out2 = root;
    s100::UpdateReport rep;
    s100::LoadError err2{s100::LoadErrorKind::Io, {}};
    const bool ok2 =
        s100::open_dataset_with_updates(p, {cell("seq", "001")}, out2, &err2, &rep);
    std::cout << "    with_updates -> ok=" << ok2 << " code=" << name(rep.code)
              << " rejected==base " << (rep.rejected == p) << " applied=" << rep.applied << "\n";
    ECHECK(!ok2 && rep.code == UpdateRejectCode::BadStructure);
    ECHECK(rep.rejected == p && rep.applied == 0);
    ECHECK(dataset_digest(out2) == root_digest);
    if (b == "base_dsed_zero") {  // oracle 8 (E6): never loaded as ED 0 / cancelled
      for (const bool r : {ok, ok2}) ECHECK(!r);
      s100::Dataset l;
      ECHECK(!s100::load_s101(p, l, nullptr));
      ECHECK(l.edition_number != 0 || (!l.cancelled && l.features.empty()));
    }
  }

  // ---- Oracle 9: session base reject ----
  std::cout << "oracle 9 (session base reject):\n";
  {
    const auto key = s100::exchange_member_key(s100::ProductId::S101, kCell);
    s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
    {
      s100::ChartSession session;
      ECHECK(session.open_exchange_set(upd_dir() / "edition" / "base_bad_cat" / "CATALOG.XML",
                                       &err));
      const bool ok = session.load_member(key, &err);
      const auto rep = session.last_update_report(key);
      std::cout << "  load_member -> " << ok << " find=" << (session.find(key) ? "member" : "null")
                << " code=" << name(rep.code) << " | " << err.message << "\n";
      ECHECK(!ok && session.find(key) == nullptr);
      ECHECK(rep.code == UpdateRejectCode::BadStructure);
    }
    {  // a member already in the map for that key stays untouched
      s100::ChartSession session;
      ECHECK(session.open_exchange_set(upd_dir() / "edition" / "base_bad_cat" / "CATALOG.XML",
                                       &err));
      ECHECK(session.insert(root, kCell) == key);
      ECHECK(!session.load_member(key, &err));
      ECHECK(session.find(key) && dataset_digest(*session.find(key)) == root_digest);
      std::cout << "  pre-existing member untouched: 1\n";
    }
  }

  // ---- Oracle 10: invariant scan ----
  std::cout << "oracle 10 (every accepted committed .000 has ED >= 1):\n";
  {
    int accepted = 0, refused = 0;
    for (const auto& e : fs::recursive_directory_iterator(s101_dir())) {
      if (!e.is_regular_file() || e.path().extension() != ".000") continue;
      s100::Dataset ds;
      s100::LoadError err{s100::LoadErrorKind::Invalid, {}};
      if (s100::open_dataset(e.path(), ds, &err)) {
        ++accepted;
        if (ds.edition_number < 1) {
          std::cerr << "FAIL accepted with ED " << ds.edition_number << ": " << e.path() << "\n";
          std::exit(1);
        }
      } else {
        ++refused;
      }
    }
    std::cout << "  accepted=" << accepted << " refused=" << refused << "\n";
    ECHECK(accepted >= 6 && refused == static_cast<int>(kBadBases.size()));
  }

  // ---- Oracle 12 (WARN): CLI viewer scenario ----
  std::cout << "oracle 12 (CLI):\n";
  {
    const auto r = run_cli(q(newed) + " --with-updates");
    std::cout << "  exit=" << r.exit_code << " " << r.out;
    if (r.exit_code != 0 || r.out.find("code=None") == std::string::npos ||
        r.out.find("superseded=1") == std::string::npos)
      std::cout << "WARN oracle 12: CLI output/exit not as expected\n";
    const auto b = run_cli(q(ed_cell("base_dsed_zero", "000")));
    std::cout << "  base_dsed_zero exit=" << b.exit_code << " " << b.err;
    ECHECK(b.exit_code != 0 && b.err.find("DSED") != std::string::npos);
  }
  // Planner request (non-blocking): an S-57 cell is named as such. No S-57 cell is committed;
  // checked against local CHS ENC_ROOT data when present (optional data, like test_exchange_set).
  {
    const fs::path enc = fs::path(S100_SOURCE_DIR) / "testdata/chs/COMBINED/ENC_ROOT";
    fs::path s57;
    if (fs::exists(enc))
      for (const auto& e : fs::recursive_directory_iterator(enc))
        if (e.is_regular_file() && e.path().extension() == ".000") {
          s57 = e.path();
          break;
        }
    if (s57.empty()) {
      std::cout << "  S-57 message: no local CHS ENC_ROOT data (optional), not exercised\n";
    } else {
      const auto r = run_cli(q(s57));
      std::cout << "  S-57 " << s57.filename().string() << " exit=" << r.exit_code << " " << r.err;
      ECHECK(r.exit_code != 0);
      ECHECK(r.err.find("not an S-101 dataset (S-57 not supported)") != std::string::npos);
    }
  }
  std::cout << "exchange_edition_hardening ok\n";
}

}  // namespace

void test_exchange_edition_hardening() {
  try {
    run();
  } catch (const std::exception& e) {  // oracle 11: nothing escapes
    std::cerr << "FAIL exception: " << e.what() << "\n";
    std::exit(1);
  }
}
