#include "s100/exchange_set.hpp"
#include "s100/session.hpp"
#include "s100/security/types.hpp"
#include "support/test_data_roots.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

#define CHECK(cond)                                                                \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
      std::exit(1);                                                                \
    }                                                                              \
  } while (0)

void test_exchange_set_synthetic_and_chs() {
  namespace fs = std::filesystem;
  const fs::path src = S100_SOURCE_DIR;
  const fs::path synthetic = src / "testdata/exchange_set/CATALOG.XML";
  CHECK(fs::exists(synthetic));

  s100::ExchangeCatalogue cat;
  s100::LoadError err;
  CHECK(s100::parse_exchange_catalogue(synthetic, cat, &err));
  CHECK(cat.datasets.size() >= 2);
  CHECK(!cat.identifier.empty());
  bool saw_0008 = false;
  for (const auto& d : cat.datasets) {
    CHECK(d.product == s100::ProductId::S101);
    if (d.file_path.filename() == "101AA00DS0008.000") {
      saw_0008 = true;
      CHECK(d.coverage.valid);
      CHECK(d.has_edition_meta);
      CHECK(d.edition_number == 8);
      CHECK(d.update_number == 0);
    }
  }
  CHECK(saw_0008);

  auto series = s100::group_cell_series(cat);
  CHECK(series.size() >= 2);

  s100::ChartSession session;
  CHECK(session.open_exchange_set(synthetic, &err, s100::security::VerifyPolicy::Off));
  const auto keys = session.member_keys();
  CHECK(keys.size() == series.size());
  // Shared session key = product + dataset identity (no bare stem).
  CHECK(keys.front().rfind("S-101:", 0) == 0);
  CHECK(session.load_member(keys.front(), &err));
  CHECK(session.find(keys.front()) != nullptr);
  CHECK(session.find(keys.front())->stats().feature_count > 0);
  CHECK(session.find(keys.front())->edition_number > 0);
  std::cout << "exchange synthetic ok members=" << keys.size()
            << " loaded=" << keys.front()
            << " features=" << session.find(keys.front())->stats().feature_count
            << " ed.upd=" << session.find(keys.front())->edition_number << "."
            << session.find(keys.front())->update_number << "\n";

  // Rule 4: open_dataset must refuse lone .001 (CHS fixture when present).
  if (s100test::chs_exercised("exchange_set.upd001")) {
    const fs::path upd001 = s100test::chs_s101_dir() / "DATASET_FILES" / "CA00" /
                            "101CA00P468N0712W" / "2" / "101CA00P468N0712W.001";
    s100::Dataset rejected;
    s100::LoadError rej_err;
    CHECK(!s100::open_dataset(upd001, rejected, &rej_err));
    CHECK(rej_err.kind == s100::LoadErrorKind::Invalid);
    CHECK(rej_err.message.find("open_dataset_with_updates") != std::string::npos);
    std::cout << "open_dataset(.001) refused: " << rej_err.message << "\n";
  }

  if (!s100test::chs_exercised("exchange_set.chs")) return;
  const fs::path chs = s100test::chs_s100_root();

  s100::ExchangeCatalogue chs_cat;
  CHECK(s100::parse_exchange_catalogue(chs, chs_cat, &err));
  CHECK(chs_cat.datasets.size() >= 3);
  auto chs_series = s100::group_cell_series(chs_cat);
  CHECK(!chs_series.empty());

  const s100::ExchangeCellSeries* with_upd = nullptr;
  for (const auto& s : chs_series) {
    if (!s.updates.empty()) {
      with_upd = &s;
      break;
    }
  }
  CHECK(with_upd != nullptr);

  s100::Dataset base_only, with_updates;
  CHECK(s100::open_dataset(with_upd->base, base_only, &err));
  CHECK(base_only.update_number == 0);
  CHECK(s100::open_cell_series(*with_upd, with_updates, &err, nullptr, s100::security::VerifyPolicy::Off));
  CHECK(with_updates.stats().feature_count != base_only.stats().feature_count);
  CHECK(with_updates.bounds().valid);
  CHECK(with_updates.edition_number == base_only.edition_number);
  CHECK(with_updates.update_number == static_cast<int>(with_upd->updates.size()));

  // ED/UPD validation: re-applying the same .001 after the series is at UPD=1 must fail.
  {
    s100::Dataset again = with_updates;
    s100::LoadError mismatch;
    CHECK(!s100::apply_s101_update(again, with_upd->updates.front(), &mismatch));
    CHECK(mismatch.kind == s100::LoadErrorKind::Invalid);
    CHECK(mismatch.message.find("ED/UPD") != std::string::npos);
    std::cout << "ED/UPD mismatch refused: " << mismatch.message << "\n";
  }

  s100::ChartSession chs_session;
  CHECK(chs_session.open_exchange_set(chs, &err, s100::security::VerifyPolicy::Off));
  const auto mem_key = s100::exchange_member_key(with_upd->product, with_upd->cell_name);
  CHECK(chs_session.load_member(mem_key, &err));
  CHECK(chs_session.find(mem_key)->stats().feature_count ==
        with_updates.stats().feature_count);
  // Bare cell_name still resolves (alias) on the shared product+id map.
  CHECK(chs_session.find(with_upd->cell_name) != nullptr);

  std::cout << "exchange chs ok cell=" << mem_key
            << " base_features=" << base_only.stats().feature_count
            << " updated_features=" << with_updates.stats().feature_count
            << " updates=" << with_upd->updates.size()
            << " final_ed.upd=" << with_updates.edition_number << "."
            << with_updates.update_number << "\n";
}
