#include "s100/exchange_set.hpp"
#include "s100/session.hpp"

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
      CHECK(d.edition_number == 1);
      CHECK(d.update_number == 0);
    }
  }
  CHECK(saw_0008);

  auto series = s100::group_cell_series(cat);
  CHECK(series.size() >= 2);

  s100::ChartSession session;
  CHECK(session.open_exchange_set(synthetic, &err));
  const auto keys = session.member_keys();
  CHECK(keys.size() == series.size());
  CHECK(session.load_member(keys.front(), &err));
  CHECK(session.find(keys.front()) != nullptr);
  CHECK(session.find(keys.front())->stats().feature_count > 0);
  std::cout << "exchange synthetic ok members=" << keys.size()
            << " loaded=" << keys.front()
            << " features=" << session.find(keys.front())->stats().feature_count << "\n";

  const char* env = std::getenv("S100_CHS_ROOT");
  fs::path chs = env ? fs::path(env) : src / "testdata/chs/COMBINED/S100_ROOT";
  if (!fs::exists(chs / "CATALOG.XML")) {
    std::cerr << "skip chs exchange: no " << chs << "\n";
    return;
  }

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
  CHECK(s100::open_cell_series(*with_upd, with_updates, &err));
  CHECK(with_updates.stats().feature_count != base_only.stats().feature_count);
  CHECK(with_updates.bounds().valid);

  s100::ChartSession chs_session;
  CHECK(chs_session.open_exchange_set(chs, &err));
  CHECK(chs_session.load_member(with_upd->cell_name, &err));
  CHECK(chs_session.find(with_upd->cell_name)->stats().feature_count ==
        with_updates.stats().feature_count);

  std::cout << "exchange chs ok cell=" << with_upd->cell_name
            << " base_features=" << base_only.stats().feature_count
            << " updated_features=" << with_updates.stats().feature_count
            << " updates=" << with_upd->updates.size() << "\n";
}
