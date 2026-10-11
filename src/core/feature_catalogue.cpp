#include "s100/feature_catalogue.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace s100 {
namespace {

// The FC is machine-generated XML with a fixed S100FC: prefix, so a small tag scanner is
// enough here and keeps s100_core free of an XML dependency. Swap for a real parser if
// catalogues from other producers need it.
std::string unescape(std::string s) {
  const std::pair<const char*, const char*> ents[] = {
      {"&amp;", "&"}, {"&lt;", "<"}, {"&gt;", ">"}, {"&quot;", "\""}, {"&apos;", "'"}};
  for (const auto& [from, to] : ents) {
    std::size_t pos = 0;
    const std::string f(from), t(to);
    while ((pos = s.find(f, pos)) != std::string::npos) {
      s.replace(pos, f.size(), t);
      pos += t.size();
    }
  }
  return s;
}

/** Text of the first <S100FC:tag>…</S100FC:tag> inside [begin,end), searching from `from`. */
bool child_text(const std::string& xml, std::size_t begin, std::size_t end, const std::string& tag,
                std::string& out, std::size_t* next = nullptr) {
  const std::string open = "<S100FC:" + tag + ">";
  const std::string close = "</S100FC:" + tag + ">";
  const std::size_t a = xml.find(open, begin);
  if (a == std::string::npos || a >= end) return false;
  const std::size_t b = xml.find(close, a);
  if (b == std::string::npos || b > end) return false;
  out = unescape(xml.substr(a + open.size(), b - a - open.size()));
  if (next) *next = b + close.size();
  return true;
}

/** Same, but only at the top level of a block: skips text nested inside child elements we
 *  don't want (attribute bindings also carry <S100FC:name>-like tags deeper down). */
std::size_t first_binding(const std::string& xml, std::size_t begin, std::size_t end) {
  std::size_t p = xml.find("<S100FC:attributeBinding", begin);
  std::size_t q = xml.find("<S100FC:featureBinding", begin);
  std::size_t r = xml.find("<S100FC:informationBinding", begin);
  std::size_t m = std::min({p, q, r});
  return m < end ? m : end;
}

}  // namespace

bool load_feature_catalogue(const std::filesystem::path& path, FeatureCatalogue& out,
                            std::string* error) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    if (error) *error = "cannot open " + path.string();
    return false;
  }
  std::stringstream ss;
  ss << in.rdbuf();
  const std::string xml = ss.str();

  out = FeatureCatalogue{};
  const std::size_t types_at = xml.find("<S100FC:S100_FC_FeatureTypes");
  const std::size_t header_end = types_at == std::string::npos ? xml.size() : types_at;
  child_text(xml, 0, header_end, "productId", out.product);
  child_text(xml, 0, header_end, "versionNumber", out.version);
  child_text(xml, 0, header_end, "versionDate", out.date);

  const std::string open = "<S100FC:S100_FC_FeatureType";
  const std::string close = "</S100FC:S100_FC_FeatureType>";
  std::size_t pos = types_at == std::string::npos ? 0 : types_at;
  while ((pos = xml.find(open, pos)) != std::string::npos) {
    // Skip the plural container tag.
    if (xml.compare(pos, open.size() + 1, open + "s") == 0) {
      pos += open.size();
      continue;
    }
    const std::size_t tag_end = xml.find('>', pos);
    const std::size_t end = xml.find(close, pos);
    if (tag_end == std::string::npos || end == std::string::npos) break;

    FcFeatureType ft;
    const std::string open_tag = xml.substr(pos, tag_end - pos);
    ft.is_abstract = open_tag.find("isAbstract=\"true\"") != std::string::npos;
    const std::size_t own_end = first_binding(xml, tag_end, end);  // header part only
    child_text(xml, tag_end, own_end, "name", ft.name);
    child_text(xml, tag_end, own_end, "code", ft.code);
    child_text(xml, tag_end, end, "featureUseType", ft.use_type);
    child_text(xml, tag_end, end, "superType", ft.super_type);
    std::size_t p = tag_end;
    std::string prim;
    while (child_text(xml, p, end, "permittedPrimitives", prim, &p)) ft.primitives.push_back(prim);

    if (!ft.code.empty()) out.feature_types[ft.code] = std::move(ft);
    pos = end + close.size();
  }

  if (out.feature_types.empty()) {
    if (error) *error = "no S100_FC_FeatureType entries in " + path.string();
    return false;
  }
  return true;
}

}  // namespace s100
