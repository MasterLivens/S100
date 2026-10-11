#include "s100/s100.hpp"
#include <chrono>
#include <cstring>
#include <iostream>

#ifndef S100_DEFAULT_FC
#define S100_DEFAULT_FC ""
#endif

// usage: s100_cli <file.000> [--fc <FC.xml>] [--layers]
int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: s100_cli <file.000> [--fc <FC.xml>] [--layers]\n";
    return 2;
  }
  std::filesystem::path path, fc_path = S100_DEFAULT_FC;
  bool list_layers = false;
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--fc") && i + 1 < argc) fc_path = argv[++i];
    else if (!std::strcmp(argv[i], "--layers")) list_layers = true;
    else path = argv[i];
  }
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

  bool classified = true;
  s100::FeatureCatalogue fc;
  std::string fc_err;
  if (!fc_path.empty() && s100::load_feature_catalogue(fc_path, fc, &fc_err)) {
    s100::ClassifyResult cr;
    classified = s100::classify_layers(ds, fc, &cr);
    for (const auto& c : cr.unknown_codes) std::cerr << "not in FC " << fc.version << ": " << c << "\n";
  } else {
    std::cerr << "warning: no Feature Catalogue (" << (fc_err.empty() ? "no path" : fc_err)
              << "), layers not classified\n";
  }

  const auto st = ds.stats();
  std::cout << "loaded " << path << " features=" << st.feature_count
            << " layers=" << st.layer_count << " ms=" << ms << "\n";
  if (list_layers)
    for (const auto& [code, li] : ds.layers.all())
      std::cout << "  " << code << " [" << li.use_type << "] " << li.feature_count << "\n";
  return st.feature_count > 0 && ms < 5000 && classified ? 0 : 1;
}
