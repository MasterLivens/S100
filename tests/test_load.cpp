#include "s100/dataset.hpp"
#include "support/test_data_roots.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#define CHECK(cond)                                                                          \
  do {                                                                                       \
    if (!(cond)) {                                                                           \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";           \
      std::exit(1);                                                                          \
    }                                                                                        \
  } while (0)

void test_load_samples() {
  namespace fs = std::filesystem;
  fs::path root = s100test::fixture_root();
  if (!fs::exists(root)) {
    std::cerr << "skip load: no " << root << "\n";
    return;
  }
  std::vector<fs::path> cells;
  for (auto& e : fs::directory_iterator(root)) {
    if (e.path().extension() == ".000") cells.push_back(e.path());
  }
  if (cells.empty()) {
    std::cerr << "skip load: no *.000 under " << root << "\n";
    return;
  }
  // Default smoke: prefer 0008
  fs::path sample = cells.front();
  for (auto& c : cells) {
    if (c.filename().string().find("0008") != std::string::npos) sample = c;
  }
  s100::Dataset ds;
  s100::LoadError err;
  const auto t0 = std::chrono::steady_clock::now();
  CHECK(s100::open_dataset(sample, ds, &err));
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0)
                      .count();
  CHECK(ms < 5000);
  CHECK(ds.stats().feature_count > 0);
  const auto b = ds.bounds();
  CHECK(b.valid);
  CHECK(b.west <= b.east);
  CHECK(b.south <= b.north);
  std::cout << "load ok " << sample << " features=" << ds.stats().feature_count
            << " ms=" << ms
            << " bounds=[" << b.west << "," << b.south << "," << b.east << "," << b.north << "]\n";
}
