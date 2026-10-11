#include "s100/exchange_set.hpp"

#include <pugixml.hpp>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <map>

namespace s100 {
namespace {

const char* local(const pugi::xml_node& n) {
  const char* name = n.name();
  const char* c = std::strchr(name, ':');
  return c ? c + 1 : name;
}

pugi::xml_node child(const pugi::xml_node& n, const char* name) {
  for (auto c : n.children())
    if (std::strcmp(local(c), name) == 0) return c;
  return {};
}

std::vector<pugi::xml_node> children(const pugi::xml_node& n, const char* name) {
  std::vector<pugi::xml_node> v;
  for (auto c : n.children())
    if (std::strcmp(local(c), name) == 0) v.push_back(c);
  return v;
}

std::string text_direct(const pugi::xml_node& n) {
  if (!n) return {};
  for (auto c : n.children()) {
    const char* ln = local(c);
    if (std::strcmp(ln, "CharacterString") == 0 || std::strcmp(ln, "Decimal") == 0)
      return c.text().as_string();
  }
  return n.text().as_string();
}

std::string text(const pugi::xml_node& n, const char* name) {
  return text_direct(child(n, name));
}

double decimal(const pugi::xml_node& n, const char* name, bool* ok = nullptr) {
  const auto t = text(n, name);
  if (t.empty()) {
    if (ok) *ok = false;
    return 0;
  }
  try {
    std::size_t idx = 0;
    const double v = std::stod(t, &idx);
    if (ok) *ok = idx > 0;
    return v;
  } catch (...) {
    if (ok) *ok = false;
    return 0;
  }
}

ProductId product_from_identifier(const std::string& id) {
  auto upper = id;
  for (char& c : upper) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  if (upper == "S-101" || upper == "S101") return ProductId::S101;
  if (upper == "S-102" || upper == "S102") return ProductId::S102;
  if (upper == "S-104" || upper == "S104") return ProductId::S104;
  if (upper == "S-111" || upper == "S111") return ProductId::S111;
  if (upper == "S-124" || upper == "S124") return ProductId::S124;
  return ProductId::Unknown;
}

std::filesystem::path resolve_file_name(const std::filesystem::path& root, std::string name) {
  if (name.rfind("file:", 0) == 0) {
    name.erase(0, 5);
    while (!name.empty() && name[0] == '/') {
      if (name.size() >= 2 && name[1] != '/') break;
      name.erase(0, 1);
    }
    if (!name.empty() && name[0] == '/') name.erase(0, 1);
  }
  std::filesystem::path rel(name);
  if (rel.is_absolute()) return rel;
  return root / rel;
}

bool is_iso8211_ext(const std::filesystem::path& p) {
  const auto ext = p.extension().string();
  return ext.size() == 4 && ext[0] == '.' && std::isdigit(static_cast<unsigned char>(ext[1])) &&
         std::isdigit(static_cast<unsigned char>(ext[2])) &&
         std::isdigit(static_cast<unsigned char>(ext[3]));
}

int extension_number(const std::filesystem::path& p) {
  const auto ext = p.extension().string();
  if (!is_iso8211_ext(p)) return -1;
  return (ext[1] - '0') * 100 + (ext[2] - '0') * 10 + (ext[3] - '0');
}

}  // namespace

bool parse_exchange_catalogue(const std::filesystem::path& path, ExchangeCatalogue& out,
                              LoadError* err) {
  namespace fs = std::filesystem;
  fs::path catalog = path;
  fs::path root = path;
  if (fs::is_directory(path)) {
    root = path;
    catalog = path / "CATALOG.XML";
    if (!fs::exists(catalog)) catalog = path / "catalog.xml";
    if (!fs::exists(catalog)) catalog = path / "Catalog.XML";
  } else {
    root = path.parent_path();
  }
  if (!fs::exists(catalog)) {
    if (err) {
      err->kind = LoadErrorKind::Io;
      err->message = "CATALOG.XML not found at " + path.string();
    }
    return false;
  }

  pugi::xml_document doc;
  const auto pref = doc.load_file(catalog.c_str());
  if (!pref) {
    if (err) {
      err->kind = LoadErrorKind::Parse;
      err->message = std::string("CATALOG.XML parse: ") + pref.description();
    }
    return false;
  }

  const auto root_node = doc.document_element();
  if (std::strcmp(local(root_node), "S100_ExchangeCatalogue") != 0) {
    if (err) {
      err->kind = LoadErrorKind::Invalid;
      err->message = "root is not S100_ExchangeCatalogue";
    }
    return false;
  }

  out = ExchangeCatalogue{};
  out.root = root;
  out.catalog_path = catalog;
  if (auto id = child(root_node, "identifier")) {
    out.identifier = text(id, "identifier");
    if (out.identifier.empty()) out.identifier = text_direct(id);
    out.date_time = text(id, "dateTime");
  }

  auto ingest_meta = [&](const pugi::xml_node& meta) {
    ExchangeDatasetEntry e;
    e.file_name = text(meta, "fileName");
    e.file_path = resolve_file_name(root, e.file_name);
    e.description = text(meta, "description");
    e.purpose = text(meta, "purpose");
    e.edition_number = child(meta, "editionNumber").text().as_int(0);
    e.update_number = child(meta, "updateNumber").text().as_int(0);
    if (auto ps = child(meta, "productSpecification")) {
      const auto pid = text(ps, "productIdentifier");
      e.product = product_from_identifier(pid.empty() ? text(ps, "name") : pid);
      e.product_version = text(ps, "version");
    }
    if (auto bb = child(meta, "boundingBox")) {
      bool ok_w = false, ok_e = false, ok_s = false, ok_n = false;
      e.coverage.west = decimal(bb, "westBoundLongitude", &ok_w);
      e.coverage.east = decimal(bb, "eastBoundLongitude", &ok_e);
      e.coverage.south = decimal(bb, "southBoundLatitude", &ok_s);
      e.coverage.north = decimal(bb, "northBoundLatitude", &ok_n);
      e.coverage.valid = ok_w && ok_e && ok_s && ok_n;
    }
    out.datasets.push_back(std::move(e));
  };

  for (auto c : root_node.children()) {
    const char* ln = local(c);
    if (std::strcmp(ln, "datasetDiscoveryMetadata") == 0) {
      auto metas = children(c, "S100_DatasetDiscoveryMetadata");
      if (metas.empty()) ingest_meta(c);
      else
        for (auto m : metas) ingest_meta(m);
    } else if (std::strcmp(ln, "S100_DatasetDiscoveryMetadata") == 0) {
      ingest_meta(c);
    }
  }

  if (out.datasets.empty()) {
    if (err) {
      err->kind = LoadErrorKind::Invalid;
      err->message = "no datasetDiscoveryMetadata in " + catalog.string();
    }
    return false;
  }
  return true;
}

std::vector<ExchangeCellSeries> group_cell_series(const ExchangeCatalogue& cat) {
  struct Acc {
    ExchangeCellSeries s;
    bool have_meta{false};
  };
  std::map<std::string, Acc> by_key;
  for (const auto& e : cat.datasets) {
    if (!is_iso8211_ext(e.file_path)) continue;
    const int n = extension_number(e.file_path);
    if (n < 0) continue;
    const std::string key = e.file_path.stem().string();
    auto& acc = by_key[key];
    if (!acc.have_meta) {
      acc.s.product = e.product != ProductId::Unknown ? e.product : ProductId::S101;
      acc.s.cell_name = key;
      acc.s.edition_number = e.edition_number;
      acc.s.coverage = e.coverage;
      acc.s.description = e.description;
      acc.have_meta = true;
    }
    if (n == 0) {
      acc.s.base = e.file_path;
      acc.s.edition_number = e.edition_number;
      if (e.coverage.valid) acc.s.coverage = e.coverage;
    } else {
      acc.s.updates.push_back(e.file_path);
    }
    acc.s.latest_update_number = std::max(acc.s.latest_update_number, e.update_number);
    if (n > acc.s.latest_update_number) acc.s.latest_update_number = n;
  }

  std::vector<ExchangeCellSeries> out;
  out.reserve(by_key.size());
  for (auto& [_, acc] : by_key) {
    if (acc.s.base.empty()) continue;
    std::sort(acc.s.updates.begin(), acc.s.updates.end(),
              [](const std::filesystem::path& a, const std::filesystem::path& b) {
                return extension_number(a) < extension_number(b);
              });
    acc.s.updates.erase(std::unique(acc.s.updates.begin(), acc.s.updates.end()),
                        acc.s.updates.end());
    out.push_back(std::move(acc.s));
  }
  std::sort(out.begin(), out.end(),
            [](const ExchangeCellSeries& a, const ExchangeCellSeries& b) {
              return a.cell_name < b.cell_name;
            });
  return out;
}

std::vector<std::filesystem::path> discover_update_files(const std::filesystem::path& base_000) {
  std::vector<std::filesystem::path> updates;
  const auto dir = base_000.parent_path();
  const auto stem = base_000.stem().string();
  if (!std::filesystem::exists(dir)) return updates;
  for (int n = 1; n <= 999; ++n) {
    char ext[8];
    std::snprintf(ext, sizeof(ext), ".%03d", n);
    const auto p = dir / (stem + ext);
    if (!std::filesystem::exists(p)) break;
    updates.push_back(p);
  }
  return updates;
}

bool open_cell_series(const ExchangeCellSeries& series, Dataset& out, LoadError* err) {
  return open_dataset_with_updates(series.base, series.updates, out, err);
}

}  // namespace s100
