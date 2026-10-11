#include "s100/s100.hpp"
#include <chrono>
#include <iostream>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: s100_cli <file.000>\n";
    return 2;
  }
  const std::filesystem::path path = argv[1];
  s100::Dataset ds;
  s100::LoadError err;
  const auto t0 = std::chrono::steady_clock::now();
  if (!s100::open_dataset(path, ds, &err)) {
    std::cerr << "load failed: " << err.message << "\n";
    return 1;
  }
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - t0)
                      .count();
  const auto st = ds.stats();
  std::cout << "loaded " << path << " features=" << st.feature_count
            << " layers=" << st.layer_count << " ms=" << ms << "\n";
  return st.feature_count > 0 && ms < 5000 ? 0 : 1;
}
