#include "s100/s100.hpp"

#include <QApplication>
#include <set>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QHash>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>
#include <QPainterPath>
#include <QPen>
#include <QSvgRenderer>
#include <QSplitter>
#include <QStatusBar>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#ifndef S100_DEFAULT_FC
#define S100_DEFAULT_FC ""
#endif
#ifndef S100_DEFAULT_PC
#define S100_DEFAULT_PC ""
#endif
#ifndef S100_DEFAULT_S102_FC
#define S100_DEFAULT_S102_FC ""
#endif
#ifndef S100_DEFAULT_S102_PC
#define S100_DEFAULT_S102_PC ""
#endif
#ifndef S100_DEFAULT_S111_FC
#define S100_DEFAULT_S111_FC ""
#endif
#ifndef S100_DEFAULT_S111_PC
#define S100_DEFAULT_S111_PC ""
#endif
#ifndef S100_DEFAULT_S124_FC
#define S100_DEFAULT_S124_FC ""
#endif
#ifndef S100_DEFAULT_S124_PC
#define S100_DEFAULT_S124_PC ""
#endif

namespace {

namespace fs = std::filesystem;

QPointF project(const QRectF& rect, double west, double south, double east, double north,
                double lon, double lat) {
  const qreal u = static_cast<qreal>((lon - west) / (east - west));
  const qreal v = static_cast<qreal>((north - lat) / (north - south));
  return QPointF(rect.left() + u * rect.width(), rect.top() + v * rect.height());
}

/** Named palette (Day / Dusk / Night) from PC ColorProfiles/colorProfile.xml — sRGB only.
 *  Token→RGB only; DrawingInstructions keep token names. */
struct ColorPalette {
  QHash<QString, QColor> by_token;

  QColor get(const std::string& token, const QColor& fallback = QColor(128, 128, 128)) const {
    const auto it = by_token.constFind(QString::fromStdString(token));
    return it == by_token.cend() ? fallback : *it;
  }

  QColor get_alpha(const std::string& token, double transparency,
                   const QColor& fallback = QColor(128, 128, 128)) const {
    QColor c = get(token, fallback);
    // Part 9a transparency: 0 = opaque, 1 = fully transparent.
    const double t = std::clamp(transparency, 0.0, 1.0);
    c.setAlphaF(1.0 - t);
    return c;
  }
};

bool load_palette(const fs::path& color_profile_xml, const QString& palette_name, ColorPalette& out,
                  QString* err = nullptr) {
  QFile f(QString::fromStdString(color_profile_xml.string()));
  if (!f.open(QIODevice::ReadOnly)) {
    if (err) *err = QString("cannot open %1").arg(f.fileName());
    return false;
  }
  QXmlStreamReader xml(&f);
  bool in_wanted = false;
  QString cur_token;
  int r = -1, g = -1, b = -1;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const auto n = xml.name();
      if (n == QLatin1String("palette")) {
        in_wanted = xml.attributes().value("name") == palette_name;
      } else if (in_wanted && n == QLatin1String("item")) {
        cur_token = xml.attributes().value("token").toString();
        r = g = b = -1;
      } else if (in_wanted && !cur_token.isEmpty()) {
        if (n == QLatin1String("red")) r = xml.readElementText().toInt();
        else if (n == QLatin1String("green")) g = xml.readElementText().toInt();
        else if (n == QLatin1String("blue")) b = xml.readElementText().toInt();
      }
    } else if (xml.isEndElement()) {
      if (xml.name() == QLatin1String("item") && in_wanted && !cur_token.isEmpty() &&
          r >= 0 && g >= 0 && b >= 0) {
        out.by_token.insert(cur_token, QColor(r, g, b));
        cur_token.clear();
      } else if (xml.name() == QLatin1String("palette")) {
        in_wanted = false;
      }
    }
  }
  if (xml.hasError()) {
    if (err) *err = xml.errorString();
    return false;
  }
  if (out.by_token.isEmpty()) {
    if (err) *err = QString("palette '%1' empty or missing").arg(palette_name);
    return false;
  }
  return true;
}

/** Named line style (ACHARE51 etc.): colour + width from LineStyles/*.xml. */
struct NamedLineStyle {
  double width_mm{0.32};
  std::string colour{"CHGRD"};
  bool has_dash{false};
};

NamedLineStyle load_named_line_style(const fs::path& line_styles_dir, const std::string& ref) {
  NamedLineStyle out;
  if (ref.empty() || ref == "_simple_") return out;
  const fs::path path = line_styles_dir / (ref + ".xml");
  QFile f(QString::fromStdString(path.string()));
  if (!f.open(QIODevice::ReadOnly)) return out;
  QXmlStreamReader xml(&f);
  bool in_pen = false;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const auto n = xml.name();
      if (n == QLatin1String("pen")) {
        in_pen = true;
        const auto w = xml.attributes().value("width");
        if (!w.isEmpty()) out.width_mm = w.toDouble();
      } else if (in_pen && n == QLatin1String("color")) {
        out.colour = xml.readElementText().toStdString();
      } else if (n == QLatin1String("dash")) {
        out.has_dash = true;
      }
    } else if (xml.isEndElement() && xml.name() == QLatin1String("pen")) {
      in_pen = false;
    }
  }
  return out;
}

/** CoverageFill bands from CoverageColor + LookupEntry state (S-102 PC). */
struct CoverageBand {
  std::string token;
  double transparency{0};
  bool has_lo{false};
  bool has_hi{false};
  double lo{0};
  double hi{0};
  enum class Interval { LtSemi, GeLt, GeSemi, Unknown } interval{Interval::Unknown};

  bool matches(float depth) const {
    switch (interval) {
      case Interval::LtSemi:
        return has_hi && depth < hi;
      case Interval::GeLt:
        return (!has_lo || depth >= lo) && (!has_hi || depth < hi);
      case Interval::GeSemi:
        return has_lo && depth >= lo;
      default:
        return false;
    }
  }
};

std::vector<CoverageBand> parse_coverage_bands(const s100::DrawingInstruction& di) {
  std::vector<CoverageBand> bands;
  std::string cur_token;
  double cur_tr = 0;
  auto split_csv = [](const std::string& s) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : s) {
      if (c == ',') {
        parts.push_back(cur);
        cur.clear();
      } else {
        cur.push_back(c);
      }
    }
    parts.push_back(cur);
    return parts;
  };
  for (const auto& el : di.coverage_state) {
    const auto colon = el.find(':');
    if (colon == std::string::npos) continue;
    const std::string cmd = el.substr(0, colon);
    const std::string rest = el.substr(colon + 1);
    const auto parts = split_csv(rest);
    if (cmd == "CoverageColor" && !parts.empty()) {
      cur_token = parts[0];
      cur_tr = parts.size() > 1 && !parts[1].empty() ? std::stod(parts[1]) : 0.0;
    } else if (cmd == "LookupEntry" && parts.size() >= 4 && !cur_token.empty()) {
      CoverageBand b;
      b.token = cur_token;
      b.transparency = cur_tr;
      if (!parts[1].empty()) {
        b.has_lo = true;
        b.lo = std::stod(parts[1]);
      }
      if (!parts[2].empty()) {
        b.has_hi = true;
        b.hi = std::stod(parts[2]);
      }
      const std::string& kind = parts[3];
      if (kind == "ltSemiInterval") b.interval = CoverageBand::Interval::LtSemi;
      else if (kind == "geLtInterval") b.interval = CoverageBand::Interval::GeLt;
      else if (kind == "geSemiInterval") b.interval = CoverageBand::Interval::GeSemi;
      bands.push_back(std::move(b));
    }
  }
  return bands;
}

/** PC AreaFills/<ref>.xml — S-100 symbolFill (v1/v2 lattice in mm).
 *  S-101 PC 2.0.0 ships only symbolFill; pixmapFill / hatchFill are not present. */
struct AreaFillDef {
  enum class Kind { None, SymbolFill };
  Kind kind{Kind::None};
  std::string symbol_ref;
  std::string area_crs{"GlobalGeometry"};  // GlobalGeometry | LocalGeometry
  double v1x{0}, v1y{0}, v2x{0}, v2y{0};
  bool clip_symbols{true};
};

AreaFillDef load_area_fill(const fs::path& area_fills_dir, const std::string& ref) {
  AreaFillDef out;
  if (ref.empty()) return out;
  const fs::path path = area_fills_dir / (ref + ".xml");
  QFile f(QString::fromStdString(path.string()));
  if (!f.open(QIODevice::ReadOnly)) return out;
  QXmlStreamReader xml(&f);
  int v_slot = 0;  // 1 = inside v1, 2 = inside v2
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      const auto n = xml.name();
      if (n == QLatin1String("symbolFill")) {
        out.kind = AreaFillDef::Kind::SymbolFill;
      } else if (n == QLatin1String("areaCRS")) {
        out.area_crs = xml.readElementText().trimmed().toStdString();
      } else if (n == QLatin1String("symbol")) {
        out.symbol_ref = xml.attributes().value("reference").toString().toStdString();
      } else if (n == QLatin1String("clipSymbols")) {
        const auto t = xml.readElementText().trimmed();
        out.clip_symbols = (t != QLatin1String("false") && t != QLatin1String("0"));
      } else if (n == QLatin1String("v1")) {
        v_slot = 1;
      } else if (n == QLatin1String("v2")) {
        v_slot = 2;
      } else if (n == QLatin1String("x") && v_slot) {
        const double v = xml.readElementText().toDouble();
        if (v_slot == 1) out.v1x = v;
        else out.v2x = v;
      } else if (n == QLatin1String("y") && v_slot) {
        const double v = xml.readElementText().toDouble();
        if (v_slot == 1) out.v1y = v;
        else out.v2y = v;
      }
    } else if (xml.isEndElement()) {
      if (xml.name() == QLatin1String("v1") || xml.name() == QLatin1String("v2")) v_slot = 0;
    }
  }
  if (out.symbol_ref.empty()) out.kind = AreaFillDef::Kind::None;
  return out;
}

qreal mm_to_px(const QPaintDevice* dev, double mm) {
  const qreal dpi = dev ? static_cast<qreal>(dev->logicalDpiX()) : 96.0;
  return static_cast<qreal>(mm) * dpi / 25.4;
}


/** View scale denominator 1:N from current geographic extent and widget width.
 *  SOURCE (IHO-style): assume 0.28 mm pixel; metres-per-pixel at mid-latitude
 *  mpp = (east-west deg) * 111320 * cos(mid_lat_rad) / width_px;
 *  view_scale_denom = round(mpp / 0.00028). Override with --view-scale. */
long compute_view_scale_denom(double west, double south, double east, double north, int width_px) {
  if (width_px <= 0) return 0;
  const double mid_lat = (south + north) * 0.5;
  const double deg_lon = east - west;
  if (!(deg_lon > 0.0)) return 0;
  constexpr double kMetresPerDegLonEq = 111320.0;
  constexpr double kPixelMetres = 0.00028;  // 0.28 mm
  const double mpp = deg_lon * kMetresPerDegLonEq * std::cos(mid_lat * 3.14159265358979323846 / 180.0) /
                     static_cast<double>(width_px);
  if (!(mpp > 0.0) || !(kPixelMetres > 0.0)) return 0;
  const long n = static_cast<long>(std::llround(mpp / kPixelMetres));
  return n > 0 ? n : 0;
}

/** Rewrite S-100 PC SVG so QtSvg sees Day (or active) palette colours.
 *  Official Symbols/*.svg use external daySvgStyle.css class tokens (fCHBLK / sCHBLK);
 *  catalogues must stay untouched — we inject an embedded <style> at load time AND promote
 *  colour class tokens to presentation attributes. Qt Svg Tiny reliably honours fill=/stroke=
 *  attributes; stroke via CSS class alone is not applied (DIAMOND1P etc. would be invisible). */
QByteArray remap_symbol_svg(const QByteArray& raw, const ColorPalette& palette) {
  QByteArray out = raw;
  // Drop xml-stylesheet PI pointing at day/dusk/night CSS (not loaded by QSvgRenderer).
  const int pi = out.indexOf("<?xml-stylesheet");
  if (pi >= 0) {
    const int pe = out.indexOf("?>", pi);
    if (pe > pi) out.remove(pi, pe - pi + 2);
  }

  // Promote fTOKEN / sTOKEN class names to fill="/stroke=" presentation attributes.
  QByteArray promoted;
  promoted.reserve(out.size() + 256);
  int i = 0;
  while (i < out.size()) {
    if (out[i] != '<') {
      promoted.append(out[i]);
      ++i;
      continue;
    }
    if (i + 1 < out.size() && (out[i + 1] == '/' || out[i + 1] == '!' || out[i + 1] == '?')) {
      const int tend = out.indexOf('>', i);
      if (tend < 0) {
        promoted.append(out.mid(i));
        break;
      }
      promoted.append(out.mid(i, tend - i + 1));
      i = tend + 1;
      continue;
    }
    const int tend = out.indexOf('>', i);
    if (tend < 0) {
      promoted.append(out.mid(i));
      break;
    }
    QByteArray tag = out.mid(i, tend - i + 1);
    const int class_key = tag.indexOf("class=\"");
    if (class_key >= 0) {
      const int c0 = class_key + 7;
      const int c1 = tag.indexOf('"', c0);
      if (c1 > c0) {
        const QByteArray classes = tag.mid(c0, c1 - c0);
        const QList<QByteArray> parts = classes.split(' ');
        QByteArray fill_hex;
        QByteArray stroke_hex;
        bool fill_none = false;
        for (const QByteArray& part : parts) {
          if (part == "f0") {
            fill_none = true;
          } else if (part.startsWith('f') && part.size() > 1) {
            const auto it = palette.by_token.constFind(QString::fromUtf8(part.mid(1)));
            if (it != palette.by_token.cend()) fill_hex = it.value().name(QColor::HexRgb).toLatin1();
          } else if (part.startsWith('s') && part.size() > 1 && part != "sl") {
            const auto it = palette.by_token.constFind(QString::fromUtf8(part.mid(1)));
            if (it != palette.by_token.cend()) stroke_hex = it.value().name(QColor::HexRgb).toLatin1();
          }
        }
        QByteArray inject;
        if (fill_none && !tag.contains("fill=")) inject += " fill=\"none\"";
        if (!fill_hex.isEmpty() && !tag.contains("fill=")) {
          inject += " fill=\"";
          inject += fill_hex;
          inject += "\"";
        }
        if (!stroke_hex.isEmpty() && !tag.contains("stroke=")) {
          inject += " stroke=\"";
          inject += stroke_hex;
          inject += "\"";
        }
        if (!inject.isEmpty()) {
          if (tag.endsWith("/>"))
            tag.insert(tag.size() - 2, inject);
          else
            tag.insert(tag.size() - 1, inject);
        }
      }
    }
    promoted.append(tag);
    i = tend + 1;
  }
  out = std::move(promoted);

  QByteArray style;
  style += "<style type=\"text/css\">";
  style += ".layout{display:none}";  // symbolBox / svgBox / pivotPoint
  style += ".f0{fill:none}";
  style += ".sl{stroke-linecap:round;stroke-linejoin:round}";
  for (auto it = palette.by_token.constBegin(); it != palette.by_token.constEnd(); ++it) {
    const QByteArray tok = it.key().toUtf8();
    const QByteArray hex = it.value().name(QColor::HexRgb).toLatin1();  // #rrggbb
    style += ".f";
    style += tok;
    style += "{fill:";
    style += hex;
    style += "}";
    style += ".s";
    style += tok;
    style += "{stroke:";
    style += hex;
    style += "}";
  }
  style += "</style>";
  const int svg = out.indexOf("<svg");
  if (svg < 0) return out;
  const int gt = out.indexOf('>', svg);
  if (gt < 0) return out;
  out.insert(gt + 1, style);
  return out;
}

/** Cached QSvgRenderer per symbol_ref (palette baked into remapped SVG). */
struct SymbolCache {
  const ColorPalette* palette{nullptr};
  fs::path symbols_dir;
  std::unordered_map<std::string, std::unique_ptr<QSvgRenderer>> by_ref;
  // Rasterized pattern tiles for AreaFill symbolFill (keyed by symbol_ref + px size).
  std::unordered_map<std::string, QImage> tile_by_key;
  // Negative cache: missing / invalid files — do not retry every paint.
  std::unordered_map<std::string, bool> missing;
  std::size_t errors{0};  // missing / invalid symbol files reported (once per ref)

  void reset(const ColorPalette* pal, fs::path dir) {
    palette = pal;
    symbols_dir = std::move(dir);
    by_ref.clear();
    tile_by_key.clear();
    missing.clear();
  }

  QSvgRenderer* get(const std::string& ref) {
    if (ref.empty() || !palette || symbols_dir.empty()) return nullptr;
    if (auto it = by_ref.find(ref); it != by_ref.end()) return it->second.get();
    if (missing.count(ref)) return nullptr;
    const fs::path path = symbols_dir / (ref + ".svg");
    QFile f(QString::fromStdString(path.string()));
    if (!f.open(QIODevice::ReadOnly)) {
      // Missing symbol is an error, not a silent drop (docs/bridge-render-contract.md hook 4).
      qWarning("error: symbol '%s' missing: %s", ref.c_str(), path.string().c_str());
      ++errors;
      missing[ref] = true;
      return nullptr;
    }
    const QByteArray remapped = remap_symbol_svg(f.readAll(), *palette);
    auto renderer = std::make_unique<QSvgRenderer>();
    if (!renderer->load(remapped) || !renderer->isValid()) {
      qWarning("error: symbol '%s' invalid SVG: %s", ref.c_str(), path.string().c_str());
      ++errors;
      missing[ref] = true;
      return nullptr;
    }
    QSvgRenderer* raw = renderer.get();
    by_ref.emplace(ref, std::move(renderer));
    return raw;
  }

  /** Rasterize Symbols/<ref>.svg at current mm→px scale for area-fill stamping. */
  const QImage* pattern_tile(const std::string& ref, qreal mm_px, QRectF* view_box_out) {
    QSvgRenderer* svg = get(ref);
    if (!svg) return nullptr;
    const QRectF vb = svg->viewBoxF();
    if (view_box_out) *view_box_out = vb;
    const int pw = std::max(1, static_cast<int>(std::ceil(std::abs(vb.width()) * mm_px)));
    const int ph = std::max(1, static_cast<int>(std::ceil(std::abs(vb.height()) * mm_px)));
    const std::string key = ref + "@" + std::to_string(pw) + "x" + std::to_string(ph);
    if (auto it = tile_by_key.find(key); it != tile_by_key.end()) return &it->second;
    QImage tile(pw, ph, QImage::Format_ARGB32_Premultiplied);
    tile.fill(Qt::transparent);
    {
      QPainter tp(&tile);
      tp.setRenderHint(QPainter::Antialiasing, true);
      svg->render(&tp, QRectF(0, 0, pw, ph));
    }
    auto [it, ok] = tile_by_key.emplace(key, std::move(tile));
    (void)ok;
    return &it->second;
  }
};


QPointF geometry_anchor(const s100::Geometry& g, const QRectF& r, double west, double south,
                        double east, double north) {
  switch (g.kind) {
    case s100::GeometryKind::Point:
      return project(r, west, south, east, north, g.point.lon, g.point.lat);
    case s100::GeometryKind::MultiPoint:
      if (!g.multipoint.empty()) {
        const auto& p = g.multipoint.front();
        return project(r, west, south, east, north, p.lon, p.lat);
      }
      break;
    case s100::GeometryKind::LineString:
      if (!g.line.empty()) {
        const auto& p = g.line[g.line.size() / 2];
        return project(r, west, south, east, north, p.lon, p.lat);
      }
      break;
    case s100::GeometryKind::Polygon:
      if (!g.polygon.empty() && !g.polygon.front().points.empty()) {
        double lon = 0, lat = 0;
        const auto& pts = g.polygon.front().points;
        for (const auto& p : pts) {
          lon += p.lon;
          lat += p.lat;
        }
        lon /= static_cast<double>(pts.size());
        lat /= static_cast<double>(pts.size());
        return project(r, west, south, east, north, lon, lat);
      }
      break;
    default:
      break;
  }
  return QPointF();
}

/** Paint geometries for one instruction: engine-resolved SpatialReference geometries when
 *  present; otherwise the feature's geometry. Viewer never walks Dataset::spatial. */
std::vector<const s100::Geometry*> paint_geometries(const s100::DrawingInstruction& di,
                                                    const s100::Feature& f) {
  std::vector<const s100::Geometry*> out;
  if (!di.geometries.empty()) {
    out.reserve(di.geometries.size());
    for (const auto& g : di.geometries) out.push_back(&g);
    return out;
  }
  // SpatialReference present but unresolved → draw nothing (do not fall back to whole feature).
  if (!di.spatial_refs.empty()) return out;
  out.push_back(&f.geometry);
  return out;
}

class MapWidget : public QWidget {
 public:
  explicit MapWidget(QWidget* parent = nullptr) : QWidget(parent) {
    setMinimumSize(640, 480);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
  }

  void setPortrayal(std::optional<s100::Dataset> ds, s100::DrawingInstructions instructions,
                    s100::PortrayalStats stats, ColorPalette palette, fs::path pc_root,
                    const s100::PortrayalCatalogue* pc) {
    dataset_ = std::move(ds);
    instructions_ = std::move(instructions);
    stats_ = std::move(stats);
    palette_ = std::move(palette);
    pc_root_ = std::move(pc_root);
    pc_ = pc;
    by_id_.clear();
    named_line_cache_.clear();
    area_fill_cache_.clear();
    symbol_cache_.reset(&palette_, pc_root_ / "Symbols");
    if (dataset_) {
      by_id_.reserve(dataset_->features.size());
      for (const auto& f : dataset_->features) by_id_[f.id] = &f;
      const auto b = dataset_->bounds();
      if (b.valid) {
        const double pad_lon = (b.east - b.west) * 0.05 + 1e-6;
        const double pad_lat = (b.north - b.south) * 0.05 + 1e-6;
        west_ = b.west - pad_lon;
        east_ = b.east + pad_lon;
        south_ = b.south - pad_lat;
        north_ = b.north + pad_lat;
      }
    }
    update();
  }

  /** Swap Day/Dusk/Night token→RGB only. Does not re-run Lua/portray; DrawingInstructions
   *  keep colour token names and paint remaps through the active palette. */
  void setPalette(ColorPalette palette) {
    palette_ = std::move(palette);
    symbol_cache_.reset(&palette_, pc_root_ / "Symbols");
    update();
  }

  const ColorPalette& palette() const { return palette_; }

  const std::optional<s100::Dataset>& dataset() const { return dataset_; }
  const s100::PortrayalStats& stats() const { return stats_; }
  s100::LayerVisibility& visibility() { return visibility_; }
  /** Viewing groups switched off (core filter_instructions_by_viewing_group semantics). */
  void set_hidden_viewing_groups(std::set<std::string> groups) {
    hidden_viewing_groups_ = std::move(groups);
    update();
  }
  void setStatusCallback(std::function<void(const QString&)> cb) { status_cb_ = std::move(cb); }
  void setTextCollisionEnabled(bool on) { text_collision_ = on; update(); }
  void setDumpTextBoxesPath(std::string path) { dump_text_boxes_path_ = std::move(path); }
  void setViewScaleOverride(long n) { view_scale_override_ = n; }  // >0 pins 1:N; 0 = compute
  void setIgnoreScaleMinimum(bool on) { ignore_scale_minimum_ = on; update(); }

  s100::Dataset* mutableDataset() { return dataset_ ? &*dataset_ : nullptr; }

  /** Replace DrawingInstructions and repaint (timeline scrubber re-portray). */
  void replaceInstructions(s100::DrawingInstructions instructions) {
    instructions_ = std::move(instructions);
    update();
  }

  /** set_active_frame + re-portray S-104 CoverageFill only; keep other DIs. */
  bool scrubWaterLevelFrame(std::size_t index) {
    if (!dataset_ || !dataset_->water_level) return false;
    if (!dataset_->water_level->set_active_frame(index)) return false;
    s100::DrawingInstructions wl_di;
    std::string err;
    if (!s100::portray_water_level(*dataset_->water_level, wl_di, &err)) return false;
    // Drop prior S-104 CoverageFill: all Coverage when no S-102 grid, else by wl feature id.
    instructions_.erase(
        std::remove_if(instructions_.begin(), instructions_.end(),
                       [this](const s100::DrawingInstruction& di) {
                         if (di.kind != s100::DrawingInstruction::Kind::Coverage) return false;
                         if (!dataset_->coverage) return true;
                         for (const auto& f : dataset_->water_level->features)
                           if (f.id == di.feature_id) return true;
                         return false;
                       }),
        instructions_.end());
    instructions_.insert(instructions_.end(), wl_di.begin(), wl_di.end());
    update();
    return true;
  }

  /** set_active_frame + re-portray S-111 Arrow DIs only; keep other DIs. */
  bool scrubCurrentFrame(std::size_t index) {
    if (!dataset_ || !dataset_->current) return false;
    if (!dataset_->current->set_active_frame(index)) return false;
    s100::DrawingInstructions cur_di;
    std::string err;
    if (!s100::portray_current(*dataset_->current, cur_di, &err)) return false;
    instructions_.erase(
        std::remove_if(instructions_.begin(), instructions_.end(),
                       [](const s100::DrawingInstruction& di) {
                         return di.kind == s100::DrawingInstruction::Kind::Arrow;
                       }),
        instructions_.end());
    instructions_.insert(instructions_.end(), cur_di.begin(), cur_di.end());
    update();
    return true;
  }

  using TimelineStepFn = std::function<bool(int delta)>;
  void setTimelineStepHandler(TimelineStepFn fn) { timeline_step_ = std::move(fn); }

  // PointLabel pairing: nearest same-feature Text within this many screen px of a Point
  // anchor (reference viewport 1280x800). docs/point-symbol-text-collision-contract.md
  static constexpr qreal kPointLabelPairPx = 48.0;

  struct TextJob {
    const s100::DrawingInstruction* di{nullptr};
    const s100::Feature* f{nullptr};
    QPointF anchor;
    QString text;
    QFont font;
    QColor color;
    int align{Qt::AlignCenter};
    QRectF box;
    bool draw{true};
    bool point_label{false};  // Kind::Text companion of Kind::Point (same feature_id)
    int plane_order{0};
    int drawing_priority{0};
  };

  struct PointAnchor {
    std::uint64_t feature_id{0};
    QPointF screen;
  };

  bool prepare_text_job(const QRectF& r, const s100::Feature& f, const s100::DrawingInstruction& di,
                        TextJob& job) const {
    if (di.text.empty()) return false;
    job.di = &di;
    job.f = &f;
    job.text = QString::fromStdString(di.text);
    job.color = palette_.get_alpha(di.text_style.font_colour, di.text_style.font_colour_transparency,
                                   palette_.get("CHBLK", Qt::black));
    job.font = font();
    const qreal px = mm_to_px(this, di.text_style.font_size > 0 ? di.text_style.font_size : 2.5);
    job.font.setPixelSize(std::max(8, static_cast<int>(std::lround(px))));
    if (di.text_style.font_weight == "Bold") job.font.setBold(true);
    if (di.text_style.font_slant == "Italics" || di.text_style.font_slant == "Italic")
      job.font.setItalic(true);
    job.align = Qt::AlignCenter;
    if (di.text_style.halign == "Start" || di.text_style.halign == "Left")
      job.align = (job.align & ~Qt::AlignHorizontal_Mask) | Qt::AlignLeft;
    else if (di.text_style.halign == "End" || di.text_style.halign == "Right")
      job.align = (job.align & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight;
    const auto geoms = paint_geometries(di, f);
    if (geoms.empty()) return false;
    const QPointF s = geometry_anchor(*geoms.front(), r, west_, south_, east_, north_);
    const qreal ox = mm_to_px(this, di.offset_x);
    const qreal oy = -mm_to_px(this, di.offset_y);
    job.anchor = QPointF(s.x() + ox, s.y() + oy);
    const QFontMetrics fm(job.font);
    const QRect br = fm.boundingRect(QRect(0, 0, 2000, 400), job.align, job.text);
    // Anchor at centre of a box sized to the text (same spirit as the old ±40×±12 box).
    const qreal w = std::max(8.0, static_cast<qreal>(br.width()) + 4.0);
    const qreal h = std::max(8.0, static_cast<qreal>(br.height()) + 2.0);
    job.box = QRectF(job.anchor.x() - w * 0.5, job.anchor.y() - h * 0.5, w, h);
    job.draw = true;
    return true;
  }

  /** Pair each Point with the nearest same-feature_id Text whose anchor is within
   *  kPointLabelPairPx; that Text becomes PointLabel. Unpaired Text stays class Text. */
  void classify_point_labels(std::vector<TextJob>& texts,
                             const std::vector<PointAnchor>& points) const {
    std::vector<char> taken(texts.size(), 0);
    for (const PointAnchor& pt : points) {
      int best = -1;
      qreal best_d2 = kPointLabelPairPx * kPointLabelPairPx;
      for (std::size_t i = 0; i < texts.size(); ++i) {
        if (taken[i] || !texts[i].di || texts[i].di->feature_id != pt.feature_id) continue;
        const qreal dx = texts[i].anchor.x() - pt.screen.x();
        const qreal dy = texts[i].anchor.y() - pt.screen.y();
        const qreal d2 = dx * dx + dy * dy;
        if (d2 <= best_d2) {
          best_d2 = d2;
          best = static_cast<int>(i);
        }
      }
      if (best >= 0) {
        texts[static_cast<std::size_t>(best)].point_label = true;
        taken[static_cast<std::size_t>(best)] = 1;
      }
    }
  }

  /** drawing priority → at equal priority unpaired Text before PointLabel → stable order. */
  void order_text_jobs_for_layout(std::vector<TextJob>& texts) const {
    std::stable_sort(texts.begin(), texts.end(), [](const TextJob& a, const TextJob& b) {
      if (a.plane_order != b.plane_order) return a.plane_order < b.plane_order;
      if (a.drawing_priority != b.drawing_priority)
        return a.drawing_priority < b.drawing_priority;
      if (a.point_label != b.point_label) return !a.point_label && b.point_label;
      return false;
    });
  }

  static bool boxes_overlap(const QRectF& a, const QRectF& b) {
    return a.intersects(b);
  }

  void layout_text_jobs(std::vector<TextJob>& texts) const {
    if (!text_collision_) return;
    // Greedy by already-sorted paint order (priority). Try small N/E/S/W/diagonal offsets; else drop.
    const qreal step = 12.0;
    const QPointF offsets[] = {
        {0, 0}, {0, -step}, {step, 0}, {0, step}, {-step, 0},
        {step, -step}, {-step, -step}, {step, step}, {-step, step},
        {0, -2 * step}, {2 * step, 0}, {0, 2 * step}, {-2 * step, 0},
    };
    std::vector<QRectF> placed;
    placed.reserve(texts.size());
    for (auto& job : texts) {
      const QRectF base = job.box;
      bool ok = false;
      for (const QPointF& off : offsets) {
        const QRectF cand = base.translated(off);
        bool hit = false;
        for (const QRectF& p : placed) {
          if (boxes_overlap(cand, p)) {
            hit = true;
            break;
          }
        }
        if (!hit) {
          job.box = cand;
          job.draw = true;
          placed.push_back(cand);
          ok = true;
          break;
        }
      }
      if (!ok) job.draw = false;  // drop label (MVP: no font shrink)
    }
  }

  void dump_text_boxes(const std::vector<TextJob>& texts) const {
    QFile f(QString::fromStdString(dump_text_boxes_path_));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return;
    QTextStream out(&f);
    out << "x,y,w,h,drawn,class,feature_id,text\n";
    for (const auto& job : texts) {
      const std::uint64_t fid = job.di ? job.di->feature_id : 0;
      out << job.box.x() << ',' << job.box.y() << ',' << job.box.width() << ',' << job.box.height()
          << ',' << (job.draw ? 1 : 0) << ','
          << (job.point_label ? "PointLabel" : "Text") << ',' << fid << ','
          << '"' << QString(job.text).replace(QLatin1Char('"'), QStringLiteral("\"\"")) << '"' << '\n';
    }
    std::fprintf(stderr, "point_glyphs_drawn=%zu\n", point_glyphs_drawn_);
  }


 protected:
  // Keyboard: Left/Right arrow keys step ±1 frame on the focused timeline product
  // (last scrubbed S-104 or S-111; see setTimelineStepHandler). Also accept [ / ].
  void keyPressEvent(QKeyEvent* e) override {
    if (timeline_step_) {
      if (e->key() == Qt::Key_Left || e->key() == Qt::Key_BracketLeft) {
        if (timeline_step_(-1)) return;
      } else if (e->key() == Qt::Key_Right || e->key() == Qt::Key_BracketRight) {
        if (timeline_step_(+1)) return;
      }
    }
    QWidget::keyPressEvent(e);
  }

  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QColor bg = palette_.by_token.isEmpty() ? QColor(235, 240, 245) : palette_.get("NODTA", QColor(235, 240, 245));
    p.fillRect(rect(), bg);
    if (!dataset_) {
      p.setPen(Qt::gray);
      p.drawText(rect(), Qt::AlignCenter, "No dataset — pass a .000 path");
      return;
    }
    if (instructions_.empty()) {
      p.setPen(Qt::gray);
      p.drawText(rect(), Qt::AlignCenter, "No DrawingInstructions — check FC/PC / portray");
      return;
    }

    const QRectF r = rect();
    // Paint order: display-plane order, then drawing priority (contract / PC).
    // SCAMIN/SCAMAX: filter by view scale before paint (docs/scamin-contract.md).
    const long view_scale =
        view_scale_override_ > 0
            ? view_scale_override_
            : compute_view_scale_denom(west_, south_, east_, north_, width());
    std::vector<const s100::DrawingInstruction*> order;
    order.reserve(instructions_.size());
    for (const auto& di : instructions_) {
      if (di.kind == s100::DrawingInstruction::Kind::Null) continue;
      // Viewing-group switch first (docs/bridge-render-contract.md hook 6): VG -> SCAMIN -> sort.
      if (!s100::visible_by_viewing_group(di, hidden_viewing_groups_)) continue;
      if (view_scale > 0 &&
          !s100::visible_at_scale(di, view_scale, ignore_scale_minimum_))
        continue;
      order.push_back(&di);
    }
    // Core draw order (docs/display-priority-contract.md): SCAMIN already filtered above.
    const std::map<std::string, int>* pc_planes =
        pc_ ? &pc_->display_planes : nullptr;
    std::stable_sort(order.begin(), order.end(),
                     [pc_planes](const s100::DrawingInstruction* a,
                                 const s100::DrawingInstruction* b) {
                       return s100::draw_order_less(*a, *b, pc_planes);
                     });

    std::vector<TextJob> texts;
    std::vector<PointAnchor> point_anchors;
    point_glyphs_drawn_ = 0;

    for (const s100::DrawingInstruction* dip : order) {
      const auto& di = *dip;
      // CoverageFill is coverage-grid backed (S-102); no Dataset::features / layer toggle.
      if (di.kind == s100::DrawingInstruction::Kind::Coverage) {
        draw_coverage(p, r, di);
        continue;
      }
      if (di.kind == s100::DrawingInstruction::Kind::Arrow) {
        draw_arrow(p, r, di);
        continue;
      }
      const auto fit = by_id_.find(di.feature_id);
      // S-124 (and similar): DI carries geometries with no Feature mirroring.
      static const s100::Feature kEmptyFeature{};
      const s100::Feature* fp = nullptr;
      if (fit != by_id_.end()) {
        const s100::Feature& f = *fit->second;
        const s100::LayerInfo* layer = dataset_->layers.find(f.feature_type);
        if (!layer) continue;  // unknown to FC: never drawn
        if (!visibility_.is_visible(*layer)) continue;
        fp = &f;
      } else if (!di.geometries.empty()) {
        fp = &kEmptyFeature;
      } else {
        continue;
      }
      const s100::Feature& f = *fp;

      switch (di.kind) {
        case s100::DrawingInstruction::Kind::Area:
          draw_area(p, r, f, di);
          break;
        case s100::DrawingInstruction::Kind::Line:
          draw_line(p, r, f, di);
          break;
        case s100::DrawingInstruction::Kind::Point: {
          // Always submit Point to painter (labels may drop; glyphs must not).
          if (di.symbol_ref.empty() && !di.fill_ref.empty() && fit == by_id_.end()) {
            // S-124 marker: filled circle in fill_ref colour (no PC symbol).
            const QColor col = palette_.get(di.fill_ref, QColor(255, 255, 0));
            p.setPen(Qt::NoPen);
            p.setBrush(col);
            for (const auto& g : di.geometries) {
              if (g.kind != s100::GeometryKind::Point) continue;
              const QPointF s = project(r, west_, south_, east_, north_, g.point.lon, g.point.lat);
              p.drawEllipse(s, 4.0, 4.0);
              point_anchors.push_back(PointAnchor{di.feature_id, s});
            }
            ++point_glyphs_drawn_;
          } else {
            draw_point(p, r, f, di);
            ++point_glyphs_drawn_;
            const qreal ox = mm_to_px(this, di.offset_x);
            const qreal oy = -mm_to_px(this, di.offset_y);
            for (const s100::Geometry* gp : paint_geometries(di, f)) {
              QPointF s;
              if (gp->kind == s100::GeometryKind::Point) {
                s = project(r, west_, south_, east_, north_, gp->point.lon, gp->point.lat);
              } else if (gp->kind == s100::GeometryKind::MultiPoint && !gp->multipoint.empty()) {
                const auto& pt = gp->multipoint.front();
                s = project(r, west_, south_, east_, north_, pt.lon, pt.lat);
              } else {
                s = geometry_anchor(*gp, r, west_, south_, east_, north_);
              }
              point_anchors.push_back(PointAnchor{di.feature_id, QPointF(s.x() + ox, s.y() + oy)});
            }
          }
          break;
        }
        case s100::DrawingInstruction::Kind::Text: {
          TextJob job;
          if (!prepare_text_job(r, f, di, job)) break;
          job.plane_order = s100::display_plane_rank(di.display_plane, pc_planes);
          job.drawing_priority = di.drawing_priority;
          texts.push_back(std::move(job));
          break;
        }
        default:
          break;
      }
    }

    classify_point_labels(texts, point_anchors);
    order_text_jobs_for_layout(texts);
    layout_text_jobs(texts);
    for (const auto& job : texts) {
      if (!job.draw) continue;
      p.setFont(job.font);
      p.setPen(job.color);
      p.drawText(job.box, job.align, job.text);
    }
    if (!dump_text_boxes_path_.empty()) dump_text_boxes(texts);
  }

  void mousePressEvent(QMouseEvent* e) override {
    const auto pick = s100::pick_wgs84(static_cast<float>(e->position().x()),
                                       static_cast<float>(e->position().y()),
                                       static_cast<float>(width()), static_cast<float>(height()),
                                       west_, south_, east_, north_);
    if (status_cb_) {
      status_cb_(QString("pick WGS84 lon=%1 lat=%2")
                     .arg(pick.lon, 0, 'f', 8)
                     .arg(pick.lat, 0, 'f', 8));
    }
  }

 private:
  void draw_arrow(QPainter& p, const QRectF& r, const s100::DrawingInstruction& di) {
    // S-111: prefer PC SCAROW*.svg via SymbolCache; else private triangle glyph.
    if (!di.symbol_ref.empty() && di.symbol_ref != "Arrow") {
      QSvgRenderer* svg = symbol_cache_.get(di.symbol_ref);
      if (svg) {
        for (const auto& g : di.geometries) {
          if (g.kind != s100::GeometryKind::Point) continue;
          const QPointF s = project(r, west_, south_, east_, north_, g.point.lon, g.point.lat);
          p.save();
          p.translate(s);
          p.rotate(di.rotation);
          const qreal scale = (di.scale_factor > 0.0 ? std::max(0.4, di.scale_factor) : 0.4) *
                              mm_to_px(this, 1.0);
          const QRectF vb = svg->viewBoxF();
          svg->render(&p, QRectF(vb.x() * scale, vb.y() * scale, vb.width() * scale, vb.height() * scale));
          p.restore();
        }
        return;
      }
    }
    // Fallback triangle (pre-PC "Arrow" glyph).
    const std::string token = di.fill_ref.empty() ? "CHGRD" : di.fill_ref;
    const QColor col = palette_.get(token, QColor(80, 80, 80));
    // Length proportional to scale_factor (speed_ms); clamp for visibility.
    const qreal speed = di.scale_factor > 0.0 ? di.scale_factor : 0.5;
    const qreal len = qBound(8.0 + speed * 14.0, 6.0, 48.0);
    const qreal head = qBound(len * 0.35, 4.0, 14.0);
    QPen pen(col, 1.5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(col);
    for (const auto& g : di.geometries) {
      if (g.kind != s100::GeometryKind::Point) continue;
      const QPointF s = project(r, west_, south_, east_, north_, g.point.lon, g.point.lat);
      p.save();
      p.translate(s);
      // direction_deg: clockwise from true N; Qt rotate is clockwise when Y grows down.
      p.rotate(di.rotation);
      // Shaft toward "from" direction (north = -Y before rotate).
      const QPointF tip(0, -len);
      const QPointF base(0, 0);
      p.drawLine(base, tip);
      QPolygonF tri;
      tri << tip << QPointF(-head * 0.45, -len + head) << QPointF(head * 0.45, -len + head);
      p.drawPolygon(tri);
      p.restore();
    }
  }

  void draw_coverage(QPainter& p, const QRectF& r, const s100::DrawingInstruction& di) {
    if (di.command != "CoverageFill") return;
    if (!dataset_) return;
    const s100::CoverageGrid* grid_ptr = nullptr;
    if (dataset_->coverage && !dataset_->coverage->grids.empty()) {
      const auto& cov = *dataset_->coverage;
      std::size_t grid_index = 0;
      bool found = false;
      for (std::size_t i = 0; i < cov.features.size(); ++i) {
        if (cov.features[i].id == di.feature_id) {
          grid_index = i < cov.grids.size() ? i : 0;
          found = true;
          break;
        }
      }
      if (!found && di.feature_id >= 1 &&
          static_cast<std::size_t>(di.feature_id) <= cov.grids.size())
        grid_index = static_cast<std::size_t>(di.feature_id - 1);
      grid_ptr = &cov.grids[grid_index];
    } else if (dataset_->water_level) {
      if (const auto* fr = dataset_->water_level->active()) grid_ptr = &fr->grid;
    }
    if (!grid_ptr) return;
    const s100::CoverageGrid& grid = *grid_ptr;
    if (grid.ncols == 0 || grid.nrows == 0 || !grid.bounds_valid) return;
    const auto bands = parse_coverage_bands(di);
    if (bands.empty()) return;

    // Rasterize at grid resolution (cap edges for huge cells), then draw into geographic envelope.
    constexpr int kMaxEdge = 2048;
    const int out_w = static_cast<int>(std::min<std::size_t>(grid.ncols, kMaxEdge));
    const int out_h = static_cast<int>(std::min<std::size_t>(grid.nrows, kMaxEdge));
    if (out_w <= 0 || out_h <= 0) return;
    QImage img(out_w, out_h, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    for (int y = 0; y < out_h; ++y) {
      const std::size_t row =
          static_cast<std::size_t>((static_cast<double>(y) + 0.5) * grid.nrows / out_h);
      for (int x = 0; x < out_w; ++x) {
        const std::size_t col =
            static_cast<std::size_t>((static_cast<double>(x) + 0.5) * grid.ncols / out_w);
        s100::CoverageSample sm;
        if (!grid.sample_cell(row, col, sm)) continue;
        const bool unc_layer = (di.coverage_attribute == "uncertainty");
        float value = 0;
        if (unc_layer) {
          if (!sm.has_uncertainty) continue;
          value = sm.uncertainty;
        } else {
          if (!sm.has_depth) continue;
          value = sm.depth;
        }
        const std::string* token = nullptr;
        double tr = 0;
        for (const auto& b : bands) {
          if (b.matches(value)) {
            token = &b.token;
            tr = b.transparency;
            break;
          }
        }
        if (!token) continue;
        const QColor c = palette_.get_alpha(*token, tr);
        img.setPixel(x, out_h - 1 - y, c.rgba());  // image row0 = north
      }
    }

    const QPointF tl = project(r, west_, south_, east_, north_, grid.west, grid.north);
    const QPointF br = project(r, west_, south_, east_, north_, grid.east, grid.south);
    const QRectF dest(tl, br);
    p.drawImage(dest.normalized(), img);
  }

  void draw_area(QPainter& p, const QRectF& r, const s100::Feature& f,
                 const s100::DrawingInstruction& di) {
    const auto geoms = paint_geometries(di, f);
    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);
    const s100::Geometry* first_poly = nullptr;
    for (const s100::Geometry* gp : geoms) {
      if (gp->kind != s100::GeometryKind::Polygon) continue;
      if (!first_poly) first_poly = gp;
      for (const auto& ring : gp->polygon) {
        QPolygonF poly;
        for (const auto& pt : ring.points)
          poly << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
        path.addPolygon(poly);
        path.closeSubpath();
      }
    }
    if (path.isEmpty()) return;

    // ColorFill: solid token from active Day/Dusk/Night palette.
    if (di.command == "ColorFill") {
      const QColor fill = palette_.get_alpha(di.fill_ref, di.fill_transparency);
      p.setBrush(fill);
      p.setPen(Qt::NoPen);
      p.drawPath(path);
      return;
    }

    // AreaFillReference / SymbolFill: tile PC AreaFills/<ref>.xml symbolFill lattice.
    if (di.command != "AreaFillReference" && di.command != "SymbolFill") return;
    const AreaFillDef& af = area_fill(di.fill_ref);
    if (af.kind != AreaFillDef::Kind::SymbolFill) return;
    const qreal mm = mm_to_px(this, 1.0);
    QRectF vb;
    const QImage* tile = symbol_cache_.pattern_tile(af.symbol_ref, mm, &vb);
    if (!tile || tile->isNull()) return;

    // Portrayal CRS y-up → screen y-down.
    const QPointF v1(af.v1x * mm, -af.v1y * mm);
    const QPointF v2(af.v2x * mm, -af.v2y * mm);
    const qreal det = v1.x() * v2.y() - v1.y() * v2.x();
    if (std::abs(det) < 1e-9) return;

    QPointF origin;
    if (af.area_crs == "LocalGeometry") {
      // Local: lattice origin at first vertex of the outer ring.
      if (first_poly && !first_poly->polygon.empty() &&
          !first_poly->polygon.front().points.empty()) {
        const auto& pt = first_poly->polygon.front().points.front();
        origin = project(r, west_, south_, east_, north_, pt.lon, pt.lat);
      } else {
        origin = path.boundingRect().topLeft();
      }
    } else {
      // GlobalGeometry: common geographic anchor so adjacent features' patterns align.
      origin = project(r, west_, south_, east_, north_, 0.0, 0.0);
    }

    // Offset from lattice pivot to tile top-left (viewBox origin).
    const QPointF tile_origin(vb.x() * mm, vb.y() * mm);
    const int pw = tile->width();
    const int ph = tile->height();

    const qreal pad =
        std::max(static_cast<qreal>(pw), static_cast<qreal>(ph)) +
        std::max(v1.manhattanLength(), v2.manhattanLength());
    const QRectF cover = path.boundingRect().adjusted(-pad, -pad, pad, pad);

    auto to_ij = [&](const QPointF& pt) {
      const qreal dx = pt.x() - origin.x();
      const qreal dy = pt.y() - origin.y();
      const qreal i = (dx * v2.y() - dy * v2.x()) / det;
      const qreal j = (v1.x() * dy - v1.y() * dx) / det;
      return QPointF(i, j);
    };

    qreal imin = 1e300, imax = -1e300, jmin = 1e300, jmax = -1e300;
    const QPointF corners[4] = {cover.topLeft(), cover.topRight(), cover.bottomLeft(),
                                cover.bottomRight()};
    for (const QPointF& c : corners) {
      const QPointF ij = to_ij(c);
      imin = std::min(imin, ij.x());
      imax = std::max(imax, ij.x());
      jmin = std::min(jmin, ij.y());
      jmax = std::max(jmax, ij.y());
    }
    const int i0 = static_cast<int>(std::floor(imin)) - 1;
    const int i1 = static_cast<int>(std::ceil(imax)) + 1;
    const int j0 = static_cast<int>(std::floor(jmin)) - 1;
    const int j1 = static_cast<int>(std::ceil(jmax)) + 1;

    p.save();
    if (af.clip_symbols) p.setClipPath(path);
    for (int j = j0; j <= j1; ++j) {
      for (int i = i0; i <= i1; ++i) {
        const QPointF pt(origin.x() + i * v1.x() + j * v2.x(),
                         origin.y() + i * v1.y() + j * v2.y());
        if (!cover.contains(pt)) continue;
        p.drawImage(pt + tile_origin, *tile);
      }
    }
    p.restore();
  }

  void draw_line(QPainter& p, const QRectF& r, const s100::Feature& f,
                 const s100::DrawingInstruction& di) {
    double width_mm = 0.32;
    std::string colour = "CHGRD";
    bool dashed = false;
    double dash_len = 2.0, gap_len = 2.0;

    if (!di.line_styles.empty()) {
      const auto& ls = di.line_styles.front();
      if (ls.width > 0) width_mm = ls.width;
      if (!ls.colour.empty()) colour = ls.colour;
      if (ls.has_dash) {
        dashed = true;
        dash_len = ls.dash_length > 0 ? ls.dash_length : 2.0;
        const double interval = ls.interval_length > 0 ? ls.interval_length : (dash_len * 2);
        gap_len = std::max(0.1, interval - dash_len);
      }
    } else if (!di.line_style_ref.empty() && di.line_style_ref != "_simple_") {
      const NamedLineStyle& ns = named_line(di.line_style_ref);
      width_mm = ns.width_mm;
      colour = ns.colour;
      dashed = ns.has_dash;
    }

    QPen pen(palette_.get(colour), mm_to_px(this, width_mm));
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    if (dashed) {
      const qreal d = mm_to_px(this, dash_len);
      const qreal g = mm_to_px(this, gap_len);
      pen.setStyle(Qt::CustomDashLine);
      pen.setDashPattern({d / pen.widthF(), g / pen.widthF()});
    }
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    for (const s100::Geometry* gp : paint_geometries(di, f)) {
      if (gp->kind == s100::GeometryKind::LineString) {
        QPolygonF line;
        for (const auto& pt : gp->line)
          line << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
        p.drawPolyline(line);
      } else if (gp->kind == s100::GeometryKind::Polygon) {
        // Area boundary (LineInstruction on a surface without SpatialReference clip).
        for (const auto& ring : gp->polygon) {
          QPolygonF poly;
          for (const auto& pt : ring.points)
            poly << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
          if (!poly.isEmpty()) poly << poly.front();
          p.drawPolyline(poly);
        }
      }
    }
  }

  void draw_point(QPainter& p, const QRectF& r, const s100::Feature& f,
                  const s100::DrawingInstruction& di) {
    // PointInstruction: PC Symbols/<symbol_ref>.svg via QSvgRenderer. CSS class tokens
    // (fCHBLK / sSNDG1 / …) are remapped through the active Day/Dusk/Night colour profile at load.
    QSvgRenderer* svg = symbol_cache_.get(di.symbol_ref);
    auto paint_one = [&](QPointF s) {
      const qreal ox = mm_to_px(this, di.offset_x);
      const qreal oy = -mm_to_px(this, di.offset_y);  // LocalOffset y up in portrayal CRS
      if (svg) {
        p.save();
        p.translate(s.x() + ox, s.y() + oy);
        if (di.rotation != 0.0) p.rotate(di.rotation);
        const qreal scale = (di.scale_factor > 0.0 ? di.scale_factor : 1.0) * mm_to_px(this, 1.0);
        // SVG user units match mm (width/height "…mm" ≈ viewBox size). Pivot is (0,0).
        const QRectF vb = svg->viewBoxF();
        svg->render(&p, QRectF(vb.x() * scale, vb.y() * scale, vb.width() * scale, vb.height() * scale));
        p.restore();
        return;
      }
      // Missing SVG file: tiny CHBLK dot (not magenta) so absent assets stay visible.
      p.setPen(Qt::NoPen);
      p.setBrush(palette_.get("CHBLK", Qt::black));
      p.drawEllipse(QPointF(s.x() + ox, s.y() + oy), 2.0, 2.0);
    };

    for (const s100::Geometry* gp : paint_geometries(di, f)) {
      if (gp->kind == s100::GeometryKind::Point) {
        paint_one(project(r, west_, south_, east_, north_, gp->point.lon, gp->point.lat));
      } else if (gp->kind == s100::GeometryKind::MultiPoint) {
        for (const auto& pt : gp->multipoint)
          paint_one(project(r, west_, south_, east_, north_, pt.lon, pt.lat));
      } else {
        // Curve SpatialReference + LinePlacement: anchor at mid-point of the referenced curve.
        paint_one(geometry_anchor(*gp, r, west_, south_, east_, north_));
      }
    }
  }

  const NamedLineStyle& named_line(const std::string& ref) {
    auto it = named_line_cache_.find(ref);
    if (it != named_line_cache_.end()) return it->second;
    NamedLineStyle ns = load_named_line_style(pc_root_ / "LineStyles", ref);
    return named_line_cache_.emplace(ref, std::move(ns)).first->second;
  }

  const AreaFillDef& area_fill(const std::string& ref) {
    auto it = area_fill_cache_.find(ref);
    if (it != area_fill_cache_.end()) return it->second;
    AreaFillDef af = load_area_fill(pc_root_ / "AreaFills", ref);
    return area_fill_cache_.emplace(ref, std::move(af)).first->second;
  }

  std::optional<s100::Dataset> dataset_;
  s100::DrawingInstructions instructions_;
  s100::PortrayalStats stats_;
  ColorPalette palette_;
  fs::path pc_root_;
  const s100::PortrayalCatalogue* pc_{nullptr};
  std::unordered_map<std::uint64_t, const s100::Feature*> by_id_;
  std::unordered_map<std::string, NamedLineStyle> named_line_cache_;
  std::unordered_map<std::string, AreaFillDef> area_fill_cache_;
  SymbolCache symbol_cache_;
  s100::LayerVisibility visibility_;  // session state, not in Dataset
  // S-102 uncertainty vendor VG 13031 may be hidden by default (docs/s102-uncertainty-contract.md U3).
  std::set<std::string> hidden_viewing_groups_{{"13031"}};
  double west_{-0.05}, south_{-0.05}, east_{0.05}, north_{0.05};
  std::function<void(const QString&)> status_cb_;
  TimelineStepFn timeline_step_;
  bool text_collision_{true};
  std::string dump_text_boxes_path_;
  std::size_t point_glyphs_drawn_{0};  // Kind::Point submitted to symbol painter this paint
  long view_scale_override_{0};        // --view-scale; 0 = compute from extent
  bool ignore_scale_minimum_{false};   // --ignore-scamin / IgnoreScaleMinimum
};

}  // namespace

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  QString cell, snapshot, s102_path, s104_path, s104_time, s111_path, s111_time, s124_path,
      dump_text_boxes, dump_layers;
  QString layers_on, layers_off;
  QString fc_path = S100_DEFAULT_FC;
  QString pc_path = S100_DEFAULT_PC;
  QString palette_name = QStringLiteral("Day");  // Day | Dusk | Night
  int s104_frame = -1;
  int s111_frame = -1;
  bool fc_set = false, pc_set = false;
  bool text_collision = true;  // default ON (docs/text-collision-contract.md)
  bool have_view_scale = false;
  long view_scale_denom = 0;
  bool ignore_scamin = false;
  for (int i = 1; i < argc; ++i) {
    const QString a = argv[i];
    if (a == "--snapshot" && i + 1 < argc) snapshot = argv[++i];
    else if (a == "--fc" && i + 1 < argc) { fc_path = argv[++i]; fc_set = true; }
    else if (a == "--pc" && i + 1 < argc) { pc_path = argv[++i]; pc_set = true; }
    else if (a == "--palette" && i + 1 < argc) palette_name = argv[++i];
    else if (a == "--s102" && i + 1 < argc) s102_path = argv[++i];
    else if (a == "--s104" && i + 1 < argc) s104_path = argv[++i];
    else if (a == "--s104-frame" && i + 1 < argc) s104_frame = QString(argv[++i]).toInt();
    else if (a == "--s104-time" && i + 1 < argc) s104_time = argv[++i];
    else if (a == "--s111" && i + 1 < argc) s111_path = argv[++i];
    else if (a == "--s111-frame" && i + 1 < argc) s111_frame = QString(argv[++i]).toInt();
    else if (a == "--s111-time" && i + 1 < argc) s111_time = argv[++i];
    else if (a == "--s124" && i + 1 < argc) s124_path = argv[++i];
    else if (a == "--no-text-collision") text_collision = false;
    else if (a == "--dump-text-boxes" && i + 1 < argc) dump_text_boxes = argv[++i];
    else if (a == "--dump-layers" && i + 1 < argc) dump_layers = argv[++i];
    else if (a == "--layers-on" && i + 1 < argc) layers_on = argv[++i];
    else if (a == "--layers-off" && i + 1 < argc) layers_off = argv[++i];
    else if (a == "--view-scale" && i + 1 < argc) {
      have_view_scale = true;
      view_scale_denom = QString(argv[++i]).toLong();
    } else if (a == "--ignore-scamin") {
      ignore_scamin = true;
    } else if (cell.isEmpty()) cell = a;
  }
  if (have_view_scale && view_scale_denom <= 0) {
    std::fprintf(stderr, "error: --view-scale must be > 0 (got %ld)\n", view_scale_denom);
    return 2;
  }
  const bool cell_is_hdf5 =
      cell.endsWith(QLatin1String(".h5"), Qt::CaseInsensitive) ||
      cell.endsWith(QLatin1String(".hdf5"), Qt::CaseInsensitive);
  // Opening an S-102 cell alone: default to S-102 FC/PC unless the user overrode paths.
  if (cell_is_hdf5) {
    if (!fc_set && QString(S100_DEFAULT_S102_FC).size() > 0) fc_path = S100_DEFAULT_S102_FC;
    if (!pc_set && QString(S100_DEFAULT_S102_PC).size() > 0) pc_path = S100_DEFAULT_S102_PC;
  }
  if (palette_name != QLatin1String("Day") && palette_name != QLatin1String("Dusk") &&
      palette_name != QLatin1String("Night")) {
    std::fprintf(stderr, "unknown --palette '%s' (use Day, Dusk, or Night)\n",
                 palette_name.toUtf8().constData());
    return 2;
  }

  QMainWindow win;
  win.setWindowTitle("s100-viewer");
  auto* central = new QWidget;
  auto* layout = new QVBoxLayout(central);
  auto* bar = new QHBoxLayout;
  auto* geo = new QCheckBox("geographic");
  auto* meta = new QCheckBox("meta");
  auto* carto = new QCheckBox("cartographic");
  geo->setChecked(true);
  meta->setChecked(false);  // meta layers off by default
  carto->setChecked(true);
  bar->addWidget(geo);
  bar->addWidget(meta);
  bar->addWidget(carto);
  bar->addWidget(new QLabel("palette:"));
  auto* palette_combo = new QComboBox;
  palette_combo->addItems({QStringLiteral("Day"), QStringLiteral("Dusk"), QStringLiteral("Night")});
  palette_combo->setCurrentText(palette_name);
  bar->addWidget(palette_combo);
  bar->addStretch();

  auto* split = new QSplitter;
  auto* layer_list = new QListWidget;
  layer_list->setMinimumWidth(220);
  auto* map = new MapWidget;
  split->addWidget(layer_list);
  split->addWidget(map);
  split->setStretchFactor(1, 1);
  layout->addLayout(bar);
  layout->addWidget(split, 1);
  win.setCentralWidget(central);
  map->setStatusCallback([&](const QString& s) { win.statusBar()->showMessage(s); });
  map->setTextCollisionEnabled(text_collision);
  if (!dump_text_boxes.isEmpty()) map->setDumpTextBoxesPath(dump_text_boxes.toStdString());
  if (have_view_scale) map->setViewScaleOverride(view_scale_denom);
  map->setIgnoreScaleMinimum(ignore_scamin);

  // FC / PC are product-level: load once per session. Context lives outside Dataset.
  s100::FeatureCatalogue fc;
  s100::PortrayalCatalogue pc;
  std::string fc_err, pc_err;
  bool have_fc = !fc_path.isEmpty() &&
                       s100::load_feature_catalogue(fc_path.toStdString(), fc, &fc_err);
  bool have_pc = !pc_path.isEmpty() &&
                       s100::load_portrayal_catalogue(pc_path.toStdString(), pc, &pc_err);

  // Colour profile path (official XML; catalogues untouched). Palette name selects which
  // Day/Dusk/Night sRGB table is used for token→RGB at paint time — portray runs once.
  fs::path color_profile_xml;
  ColorPalette palette;
  QString pal_err;
  if (have_pc) {
    color_profile_xml = pc.root / "ColorProfiles" / "colorProfile.xml";
    if (!pc.color_profiles.empty() && !pc.color_profiles.front().file_name.empty())
      color_profile_xml = pc.root / "ColorProfiles" / pc.color_profiles.front().file_name;
    if (!load_palette(color_profile_xml, palette_name, palette, &pal_err))
      std::fprintf(stderr, "colour profile: %s\n", pal_err.toUtf8().constData());
  }

  auto apply_palette = [&](const QString& name) {
    if (!have_pc || color_profile_xml.empty()) return;
    ColorPalette next;
    QString err;
    if (!load_palette(color_profile_xml, name, next, &err)) {
      std::fprintf(stderr, "colour profile: %s\n", err.toUtf8().constData());
      return;
    }
    palette = std::move(next);
    palette_name = name;
    map->setPalette(palette);  // remaps SVG/cache; does not re-portray
  };
  QObject::connect(palette_combo, &QComboBox::currentTextChanged, map, [&](const QString& name) {
    apply_palette(name);
  });

  auto refresh_list = [&]() {
    layer_list->blockSignals(true);
    layer_list->clear();
    if (map->dataset()) {
      for (const auto& [code, li] : map->dataset()->layers.all()) {
        auto* item = new QListWidgetItem(QString("%1 (%2) — %3")
                                             .arg(QString::fromStdString(li.code))
                                             .arg(static_cast<qulonglong>(li.feature_count))
                                             .arg(QString::fromStdString(li.use_type)));
        item->setData(Qt::UserRole, QString::fromStdString(li.code));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(map->visibility().is_visible(li) ? Qt::Checked : Qt::Unchecked);
        layer_list->addItem(item);
      }
    }
    layer_list->blockSignals(false);
  };

  auto use_toggle = [&](QCheckBox* box, const char* use) {
    QObject::connect(box, &QCheckBox::toggled, map, [&, use](bool on) {
      map->visibility().set_use_type(use, on);
      if (map->dataset())
        for (const auto& [code, li] : map->dataset()->layers.all())
          if (li.use_type == use) map->visibility().clear_layer_rule(code);
      refresh_list();
      map->update();
    });
  };
  use_toggle(geo, "geographic");
  use_toggle(meta, "meta");
  use_toggle(carto, "cartographic");
  QObject::connect(layer_list, &QListWidget::itemChanged, map, [map](QListWidgetItem* item) {
    map->visibility().set_layer(item->data(Qt::UserRole).toString().toStdString(),
                                item->checkState() == Qt::Checked);
    map->update();
  });

  if (!have_fc) {
    win.statusBar()->showMessage(
        QString("Feature Catalogue not loaded (%1) — pass --fc <S-101 FC xml>")
            .arg(QString::fromStdString(fc_err.empty() ? "no path" : fc_err)));
  } else if (!have_pc) {
    win.statusBar()->showMessage(
        QString("Portrayal Catalogue not loaded (%1) — pass --pc <PC dir>")
            .arg(QString::fromStdString(pc_err.empty() ? "no path" : pc_err)));
  } else if (!cell.isEmpty()) {
    s100::Dataset ds;
    s100::LoadError err;
    // docs/update-robustness-contract.md (viewer, thin): a base .000 with sibling updates goes
    // through the core update path; on a rejected update the core leaves base + applied prefix
    // in ds, which is drawn with a status-bar warning. The viewer never decides sequencing.
    QString update_warning;
    auto open_cell = [&]() -> bool {
      const std::filesystem::path p = cell.toStdString();
      if (p.extension() == ".000") {
        const auto updates = s100::discover_update_files(p);
        if (!updates.empty()) {
          s100::UpdateReport rep;
          const bool ok = s100::open_dataset_with_updates(p, updates, ds, &err, &rep);
          // edition-hardening E1: old-edition leftovers are skipped by the core; just say so.
          if (!rep.superseded.empty())
            update_warning = QString("NOTICE: %1 superseded update(s) can be removed — ")
                                 .arg(static_cast<qulonglong>(rep.superseded.size()));
          if (ok) return true;
          if (rep.code == s100::UpdateRejectCode::None || rep.rejected == p) return false;
          update_warning += QString("WARNING: update rejected (%1, code=%2) — showing last good "
                                   "state %3.%4 (applied=%5) — ")
                               .arg(QString::fromStdString(rep.rejected.filename().string()))
                               .arg(QString::fromLatin1(s100::update_reject_code_name(rep.code)))
                               .arg(ds.edition_number)
                               .arg(ds.update_number)
                               .arg(rep.applied);
          return true;
        }
      }
      return s100::open_dataset(p, ds, &err);
    };
    // docs/cancel-reissue-contract.md: a cancelled dataset is not drawn; status bar says so.
    auto open_drawable = [&]() -> bool {
      if (!open_cell()) return false;
      if (!ds.cancelled) return true;
      err.message = "CANCELLED: " + std::filesystem::path(cell.toStdString()).stem().string();
      return false;
    };
    if (open_drawable()) {
      s100::DrawingInstructions instructions;
      s100::PortrayalStats ps;
      QString msg;
      bool portrayed = false;

      if (ds.product == s100::ProductId::S102 && ds.coverage) {
        // S-102 alone: CoverageFill instructions from coverage host.
        std::string perr;
        auto ctx = s100::PortrayalContext::defaults(pc);
        if (s100::portray(*ds.coverage, fc, pc, ctx, instructions, &perr, &ps)) {
          portrayed = true;
          msg = QString("loaded S-102 %1 — coverage %2x%3 instructions=%4 lua_errors=%5 (FC %6 %7, PC %8 %9)")
                    .arg(cell)
                    .arg(static_cast<qulonglong>(ds.coverage->grid().ncols))
                    .arg(static_cast<qulonglong>(ds.coverage->grid().nrows))
                    .arg(static_cast<qulonglong>(ps.instructions))
                    .arg(static_cast<qulonglong>(ps.lua_errors))
                    .arg(QString::fromStdString(fc.product))
                    .arg(QString::fromStdString(fc.version))
                    .arg(QString::fromStdString(pc.product))
                    .arg(QString::fromStdString(pc.version));
        } else {
          msg = QString("S-102 portray failed: %1").arg(QString::fromStdString(perr));
        }
      } else if (ds.product == s100::ProductId::S104 && ds.water_level) {
        // S-104 alone: tokenized CoverageFill for active frame (PC deferred).
        if (s104_frame >= 0) ds.water_level->set_active_frame(static_cast<std::size_t>(s104_frame));
        if (!s104_time.isEmpty()) ds.water_level->set_active_time(s104_time.toStdString());
        std::string perr;
        if (s100::portray_water_level(*ds.water_level, instructions, &perr)) {
          portrayed = true;
          ps.instructions = instructions.size();
          const auto* fr = ds.water_level->active();
          msg = QString("loaded S-104 %1 — frames=%2 active=%3 time=%4 instructions=%5")
                    .arg(cell)
                    .arg(static_cast<qulonglong>(ds.water_level->frames.size()))
                    .arg(static_cast<qulonglong>(ds.water_level->active_frame))
                    .arg(fr ? QString::fromStdString(fr->time_iso) : QStringLiteral("?"))
                    .arg(static_cast<qulonglong>(instructions.size()));
        } else {
          msg = QString("S-104 portray failed: %1").arg(QString::fromStdString(perr));
        }
      } else if (ds.product == s100::ProductId::S111 && ds.current) {
        // S-111 alone: Arrow DI with SCAROW* from PC bands; load pinned FC+PC + ColorProfiles.
        if (!fc_set && QString(S100_DEFAULT_S111_FC).size() > 0) {
          std::string e111;
          if (s100::load_feature_catalogue(std::string(S100_DEFAULT_S111_FC), fc, &e111)) {
            fc_path = S100_DEFAULT_S111_FC;
            have_fc = true;
          }
        }
        if (!pc_set && QString(S100_DEFAULT_S111_PC).size() > 0) {
          std::string e111;
          if (s100::load_portrayal_catalogue(std::string(S100_DEFAULT_S111_PC), pc, &e111)) {
            pc_path = S100_DEFAULT_S111_PC;
            have_pc = true;
            color_profile_xml = pc.root / "ColorProfiles" / "colorProfile.xml";
            if (!pc.color_profiles.empty() && !pc.color_profiles.front().file_name.empty())
              color_profile_xml = pc.root / "ColorProfiles" / pc.color_profiles.front().file_name;
            QString perr_pal;
            load_palette(color_profile_xml, palette_name, palette, &perr_pal);
          }
        }
        if (s111_frame >= 0) ds.current->set_active_frame(static_cast<std::size_t>(s111_frame));
        if (!s111_time.isEmpty()) ds.current->set_active_time(s111_time.toStdString());
        std::string perr;
        if (s100::portray_current(*ds.current, instructions, &perr)) {
          portrayed = true;
          ps.instructions = instructions.size();
          const auto* fr = ds.current->active();
          msg = QString("loaded S-111 %1 — frames=%2 active=%3 time=%4 arrows=%5 (PC SCAROW)")
                    .arg(cell)
                    .arg(static_cast<qulonglong>(ds.current->frames.size()))
                    .arg(static_cast<qulonglong>(ds.current->active_frame))
                    .arg(fr ? QString::fromStdString(fr->time_iso) : QStringLiteral("?"))
                    .arg(static_cast<qulonglong>(instructions.size()));
        } else {
          msg = QString("S-111 portray failed: %1").arg(QString::fromStdString(perr));
        }
      } else if (ds.product == s100::ProductId::S124 && ds.nav_warnings) {
        // S-124 alone: prefer official PC via portray_nav_warnings_pc; heuristic only on fail.
        if (!fc_set && QString(S100_DEFAULT_S124_FC).size() > 0) {
          std::string e124;
          if (s100::load_feature_catalogue(std::string(S100_DEFAULT_S124_FC), fc, &e124)) {
            fc_path = S100_DEFAULT_S124_FC;
            have_fc = true;
          }
        }
        if (!pc_set && QString(S100_DEFAULT_S124_PC).size() > 0) {
          std::string e124;
          if (s100::load_portrayal_catalogue(std::string(S100_DEFAULT_S124_PC), pc, &e124)) {
            pc_path = S100_DEFAULT_S124_PC;
            have_pc = true;
            color_profile_xml = pc.root / "ColorProfiles" / "colorProfile.xml";
            if (!pc.color_profiles.empty() && !pc.color_profiles.front().file_name.empty())
              color_profile_xml = pc.root / "ColorProfiles" / pc.color_profiles.front().file_name;
            QString perr_pal;
            load_palette(color_profile_xml, palette_name, palette, &perr_pal);
          }
        }
        std::string perr;
        bool ok = false;
        if (have_fc && have_pc) {
          ok = s100::portray_nav_warnings_pc(*ds.nav_warnings, fc, pc, instructions, &ps, &perr);
        }
        if (!ok) {
          // Explicit fail fallback to heuristic CHYLW path (documented).
          perr.clear();
          instructions.clear();
          ok = s100::portray_nav_warnings(*ds.nav_warnings, instructions, &perr);
          if (ok) perr = "heuristic fallback after PC fail";
        }
        if (ok) {
          portrayed = true;
          ps.instructions = instructions.size();
          msg = QString("loaded S-124 %1 — warnings=%2 instructions=%3")
                    .arg(cell)
                    .arg(static_cast<qulonglong>(ds.nav_warnings->warnings.size()))
                    .arg(static_cast<qulonglong>(instructions.size()));
        } else {
          msg = QString("S-124 portray failed: %1").arg(QString::fromStdString(perr));
        }
      } else {
        s100::ClassifyResult cr;
        const bool ok = s100::classify_layers(ds, fc, &cr);
        s100::PortrayalSession session(fc, pc);
        std::string perr;
        const auto* di = session.portray(ds, &ps, &perr);
        if (di) {
          instructions = *di;
          portrayed = true;
          // Honour PC context IgnoreScaleMinimum when portray session has it.
          auto it = session.context().values.find("IgnoreScaleMinimum");
          if (it != session.context().values.end() &&
              (it->second == "true" || it->second == "True" || it->second == "1"))
            ignore_scamin = true;
          msg = QString("loaded %1 — features=%2 instructions=%3 lua_errors=%4 (FC %5 %6, PC %7 %8)")
                    .arg(cell)
                    .arg(static_cast<qulonglong>(ps.features))
                    .arg(static_cast<qulonglong>(ps.instructions))
                    .arg(static_cast<qulonglong>(ps.lua_errors))
                    .arg(QString::fromStdString(fc.product))
                    .arg(QString::fromStdString(fc.version))
                    .arg(QString::fromStdString(pc.product))
                    .arg(QString::fromStdString(pc.version));
          if (!ok)
            msg += QString(" — WARNING: %1 codes not in FC, not drawn")
                       .arg(static_cast<qulonglong>(cr.unknown_codes.size()));
        } else {
          msg = QString("portray failed: %1").arg(QString::fromStdString(perr));
        }

        // S-102 overlay via one ChartSession (contract): S-101 + S-102 keys, union bounds.
        if (portrayed && !s102_path.isEmpty()) {
          s100::ChartSession chart;
          s100::LoadError cerr;
          std::string k101, k102;
          // Keep the already-portrayed S-101 Dataset; register both under session keys.
          k101 = chart.insert(ds);
          if (!chart.open_and_insert(s102_path.toStdString(), &cerr, &k102) ||
              !chart.find(k102) || !chart.find(k102)->coverage) {
            msg += QString(" — S-102 overlay failed: %1")
                       .arg(QString::fromStdString(cerr.message.empty() ? "no coverage" : cerr.message));
          } else {
            s100::FeatureCatalogue fc102;
            s100::PortrayalCatalogue pc102;
            std::string e102;
            const QString fc102_path =
                QString(S100_DEFAULT_S102_FC).isEmpty() ? fc_path : QString(S100_DEFAULT_S102_FC);
            const QString pc102_path =
                QString(S100_DEFAULT_S102_PC).isEmpty() ? pc_path : QString(S100_DEFAULT_S102_PC);
            if (s100::load_feature_catalogue(fc102_path.toStdString(), fc102, &e102) &&
                s100::load_portrayal_catalogue(pc102_path.toStdString(), pc102, &e102)) {
              s100::DrawingInstructions cov_di;
              s100::PortrayalStats cps;
              auto ctx = s100::PortrayalContext::defaults(pc102);
              const auto& cov = *chart.find(k102)->coverage;
              if (s100::portray(cov, fc102, pc102, ctx, cov_di, &e102, &cps)) {
                ds.coverage = cov;  // paint + Dataset::bounds unions feature+coverage envelopes
                instructions.insert(instructions.end(), cov_di.begin(), cov_di.end());
                const auto ub = ds.bounds();
                msg += QString(" — overlay %1 + %2 (CoverageFill +%3, bounds union [%4,%5]–[%6,%7])")
                           .arg(QString::fromStdString(k101))
                           .arg(QString::fromStdString(k102))
                           .arg(static_cast<qulonglong>(cov_di.size()))
                           .arg(ub.west, 0, 'f', 5)
                           .arg(ub.south, 0, 'f', 5)
                           .arg(ub.east, 0, 'f', 5)
                           .arg(ub.north, 0, 'f', 5);
              } else {
                msg += QString(" — S-102 portray failed: %1").arg(QString::fromStdString(e102));
              }
            } else {
              msg += QString(" — S-102 FC/PC: %1").arg(QString::fromStdString(e102));
            }
          }
        }

        // S-104 overlay via ChartSession (contract): S-101 + S-104 keys, union bounds.
        if (portrayed && !s104_path.isEmpty()) {
          s100::ChartSession chart;
          s100::LoadError cerr;
          std::string k101, k104;
          k101 = chart.insert(ds);
          if (!chart.open_and_insert(s104_path.toStdString(), &cerr, &k104) ||
              !chart.find(k104) || !chart.find(k104)->water_level) {
            msg += QString(" — S-104 overlay failed: %1")
                       .arg(QString::fromStdString(cerr.message.empty() ? "no water_level"
                                                                        : cerr.message));
          } else {
            auto& wl = *chart.find(k104)->water_level;
            if (s104_frame >= 0) wl.set_active_frame(static_cast<std::size_t>(s104_frame));
            if (!s104_time.isEmpty()) wl.set_active_time(s104_time.toStdString());
            s100::DrawingInstructions wl_di;
            std::string e104;
            if (s100::portray_water_level(wl, wl_di, &e104)) {
              ds.water_level = wl;  // paint + Dataset::bounds unions water_level envelope
              instructions.insert(instructions.end(), wl_di.begin(), wl_di.end());
              const auto ub = ds.bounds();
              msg += QString(" — overlay %1 + %2 (S-104 CoverageFill +%3 frame=%4, bounds union "
                             "[%5,%6]–[%7,%8])")
                         .arg(QString::fromStdString(k101))
                         .arg(QString::fromStdString(k104))
                         .arg(static_cast<qulonglong>(wl_di.size()))
                         .arg(static_cast<qulonglong>(wl.active_frame))
                         .arg(ub.west, 0, 'f', 5)
                         .arg(ub.south, 0, 'f', 5)
                         .arg(ub.east, 0, 'f', 5)
                         .arg(ub.north, 0, 'f', 5);
            } else {
              msg += QString(" — S-104 portray failed: %1").arg(QString::fromStdString(e104));
            }
          }
        }

        // S-111 overlay via ChartSession (contract): S-101 + S-111 keys, union bounds.
        if (portrayed && !s111_path.isEmpty()) {
          s100::ChartSession chart;
          s100::LoadError cerr;
          std::string k101, k111;
          k101 = chart.insert(ds);
          if (!chart.open_and_insert(s111_path.toStdString(), &cerr, &k111) ||
              !chart.find(k111) || !chart.find(k111)->current) {
            msg += QString(" — S-111 overlay failed: %1")
                       .arg(QString::fromStdString(cerr.message.empty() ? "no current"
                                                                        : cerr.message));
          } else {
            auto& cur = *chart.find(k111)->current;
            if (s111_frame >= 0) cur.set_active_frame(static_cast<std::size_t>(s111_frame));
            if (!s111_time.isEmpty()) cur.set_active_time(s111_time.toStdString());
            s100::DrawingInstructions cur_di;
            std::string e111;
            if (s100::portray_current(cur, cur_di, &e111)) {
              ds.current = cur;  // paint + Dataset::bounds unions current envelope
              instructions.insert(instructions.end(), cur_di.begin(), cur_di.end());
              const auto ub = ds.bounds();
              msg += QString(" — overlay %1 + %2 (S-111 Arrow +%3 frame=%4, bounds union "
                             "[%5,%6]–[%7,%8])")
                         .arg(QString::fromStdString(k101))
                         .arg(QString::fromStdString(k111))
                         .arg(static_cast<qulonglong>(cur_di.size()))
                         .arg(static_cast<qulonglong>(cur.active_frame))
                         .arg(ub.west, 0, 'f', 5)
                         .arg(ub.south, 0, 'f', 5)
                         .arg(ub.east, 0, 'f', 5)
                         .arg(ub.north, 0, 'f', 5);
            } else {
              msg += QString(" — S-111 portray failed: %1").arg(QString::fromStdString(e111));
            }
          }
        }

        // S-124 overlay via ChartSession (contract): S-101 + S-124 keys, union bounds.
        if (portrayed && !s124_path.isEmpty()) {
          s100::ChartSession chart;
          s100::LoadError cerr;
          std::string k101, k124;
          k101 = chart.insert(ds);
          if (!chart.open_and_insert(s124_path.toStdString(), &cerr, &k124) ||
              !chart.find(k124) || !chart.find(k124)->nav_warnings) {
            msg += QString(" — S-124 overlay failed: %1")
                       .arg(QString::fromStdString(cerr.message.empty() ? "no nav_warnings"
                                                                        : cerr.message));
          } else {
            const auto& nw = *chart.find(k124)->nav_warnings;
            s100::DrawingInstructions nw_di;
            std::string e124;
            bool nw_ok = false;
            {
              s100::FeatureCatalogue fc124;
              s100::PortrayalCatalogue pc124;
              std::string e_cat;
              const std::string fc124p = S100_DEFAULT_S124_FC;
              const std::string pc124p = S100_DEFAULT_S124_PC;
              if (!fc124p.empty() && !pc124p.empty() &&
                  s100::load_feature_catalogue(fc124p, fc124, &e_cat) &&
                  s100::load_portrayal_catalogue(pc124p, pc124, &e_cat)) {
                nw_ok = s100::portray_nav_warnings_pc(nw, fc124, pc124, nw_di, nullptr, &e124);
              }
            }
            if (!nw_ok) {
              e124.clear();
              nw_di.clear();
              nw_ok = s100::portray_nav_warnings(nw, nw_di, &e124);
            }
            if (nw_ok) {
              ds.nav_warnings = nw;  // paint + Dataset::bounds unions nav_warnings envelope
              instructions.insert(instructions.end(), nw_di.begin(), nw_di.end());
              const auto ub = ds.bounds();
              msg += QString(" — overlay %1 + %2 (S-124 warnings=%3 DI +%4, bounds union "
                             "[%5,%6]–[%7,%8])")
                         .arg(QString::fromStdString(k101))
                         .arg(QString::fromStdString(k124))
                         .arg(static_cast<qulonglong>(nw.warnings.size()))
                         .arg(static_cast<qulonglong>(nw_di.size()))
                         .arg(ub.west, 0, 'f', 5)
                         .arg(ub.south, 0, 'f', 5)
                         .arg(ub.east, 0, 'f', 5)
                         .arg(ub.north, 0, 'f', 5);
            } else {
              msg += QString(" — S-124 portray failed: %1").arg(QString::fromStdString(e124));
            }
          }
        }
      }

      map->setIgnoreScaleMinimum(ignore_scamin);
      map->setPortrayal(std::move(ds), portrayed ? std::move(instructions) : s100::DrawingInstructions{},
                        ps, palette, pc.root, &pc);
      auto apply_layer_csv = [&](const QString& csv, bool on) {
        for (const QString& part : csv.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
          const QString code = part.trimmed();
          if (!code.isEmpty()) map->visibility().set_layer(code.toStdString(), on);
        }
      };
      apply_layer_csv(layers_on, true);
      apply_layer_csv(layers_off, false);
      refresh_list();
      win.statusBar()->showMessage(update_warning + msg);
    } else {
      win.statusBar()->showMessage(QString::fromStdString(err.message));
    }
  }

  // Timeline scrubbers for S-104 / S-111 (docs/timeline-ui-contract.md). Independent strips.
  QSlider* s104_slider = nullptr;
  QLabel* s104_time_label = nullptr;
  QSlider* s111_slider = nullptr;
  QLabel* s111_time_label = nullptr;
  // Which product Left/Right keys step: 104 or 111 (last scrubbed wins).
  int timeline_focus_product = 0;

  auto update_s104_time_label = [&]() {
    if (!s104_time_label) return;
    auto* ds = map->mutableDataset();
    if (!ds || !ds->water_level || ds->water_level->frames.empty()) return;
    const auto& wl = *ds->water_level;
    const std::size_t i = wl.active_frame;
    const std::size_t n = wl.frames.size();
    QString t;
    if (i < n && !wl.frames[i].time_iso.empty())
      t = QString::fromStdString(wl.frames[i].time_iso);
    else
      t = QString("frame %1/%2").arg(static_cast<qulonglong>(i)).arg(static_cast<qulonglong>(n));
    s104_time_label->setText(QString("S-104 %1").arg(t));
  };
  auto update_s111_time_label = [&]() {
    if (!s111_time_label) return;
    auto* ds = map->mutableDataset();
    if (!ds || !ds->current || ds->current->frames.empty()) return;
    const auto& cur = *ds->current;
    const std::size_t i = cur.active_frame;
    const std::size_t n = cur.frames.size();
    QString t;
    if (i < n && !cur.frames[i].time_iso.empty())
      t = QString::fromStdString(cur.frames[i].time_iso);
    else
      t = QString("frame %1/%2").arg(static_cast<qulonglong>(i)).arg(static_cast<qulonglong>(n));
    s111_time_label->setText(QString("S-111 %1").arg(t));
  };

  if (map->dataset() && map->dataset()->water_level &&
      !map->dataset()->water_level->frames.empty()) {
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("S-104:"));
    s104_slider = new QSlider(Qt::Horizontal);
    const int n = static_cast<int>(map->dataset()->water_level->frames.size());
    s104_slider->setRange(0, n - 1);
    s104_slider->setValue(static_cast<int>(map->dataset()->water_level->active_frame));
    s104_slider->setTickPosition(QSlider::TicksBelow);
    s104_slider->setTickInterval(1);
    s104_time_label = new QLabel;
    row->addWidget(s104_slider, 1);
    row->addWidget(s104_time_label);
    layout->addLayout(row);
    update_s104_time_label();
    QObject::connect(s104_slider, &QSlider::valueChanged, map, [&, map](int v) {
      timeline_focus_product = 104;
      map->scrubWaterLevelFrame(static_cast<std::size_t>(v));
      update_s104_time_label();
      map->setFocus();
    });
  }
  if (map->dataset() && map->dataset()->current && !map->dataset()->current->frames.empty()) {
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("S-111:"));
    s111_slider = new QSlider(Qt::Horizontal);
    const int n = static_cast<int>(map->dataset()->current->frames.size());
    s111_slider->setRange(0, n - 1);
    s111_slider->setValue(static_cast<int>(map->dataset()->current->active_frame));
    s111_slider->setTickPosition(QSlider::TicksBelow);
    s111_slider->setTickInterval(1);
    s111_time_label = new QLabel;
    row->addWidget(s111_slider, 1);
    row->addWidget(s111_time_label);
    layout->addLayout(row);
    update_s111_time_label();
    QObject::connect(s111_slider, &QSlider::valueChanged, map, [&, map](int v) {
      timeline_focus_product = 111;
      map->scrubCurrentFrame(static_cast<std::size_t>(v));
      update_s111_time_label();
      map->setFocus();
    });
  }
  if (s104_slider && !s111_slider) timeline_focus_product = 104;
  if (s111_slider && !s104_slider) timeline_focus_product = 111;
  if (s104_slider && s111_slider) timeline_focus_product = 104;  // default focus S-104

  // Keyboard Left/Right (and [ / ]) → timeline_step_frame on focused product.
  map->setTimelineStepHandler([&, map](int delta) -> bool {
    auto* ds = map->mutableDataset();
    if (!ds) return false;
    int focus = timeline_focus_product;
    if (focus == 0) {
      if (ds->water_level) focus = 104;
      else if (ds->current) focus = 111;
    }
    if (focus == 104 && ds->water_level) {
      if (!s100::timeline_step_frame(*ds->water_level, delta)) return false;
      if (s104_slider) {
        s104_slider->blockSignals(true);
        s104_slider->setValue(static_cast<int>(ds->water_level->active_frame));
        s104_slider->blockSignals(false);
      }
      map->scrubWaterLevelFrame(ds->water_level->active_frame);
      // scrubWaterLevelFrame set_active_frame again (no-op same index) + re-portray
      update_s104_time_label();
      return true;
    }
    if (focus == 111 && ds->current) {
      if (!s100::timeline_step_frame(*ds->current, delta)) return false;
      if (s111_slider) {
        s111_slider->blockSignals(true);
        s111_slider->setValue(static_cast<int>(ds->current->active_frame));
        s111_slider->blockSignals(false);
      }
      map->scrubCurrentFrame(ds->current->active_frame);
      update_s111_time_label();
      return true;
    }
    return false;
  });

  win.resize(1280, 800);
  win.show();

  // Headless: --snapshot / --dump-text-boxes / --dump-layers (QT_QPA_PLATFORM=offscreen)
  if (!snapshot.isEmpty() || !dump_text_boxes.isEmpty() || !dump_layers.isEmpty()) {
    QTimer::singleShot(200, [&win, map, snapshot, dump_layers]() {
      bool ok = true;
      if (!snapshot.isEmpty()) {
        ok = win.grab().save(snapshot);
        std::fprintf(ok ? stdout : stderr, "snapshot %s %s\n", ok ? "saved" : "FAILED",
                     snapshot.toUtf8().constData());
      } else {
        (void)win.grab();  // force paint → dump-text-boxes
      }
      if (ok && !dump_layers.isEmpty()) {
        QFile f(dump_layers);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
          std::fprintf(stderr, "dump-layers FAILED %s\n", dump_layers.toUtf8().constData());
          ok = false;
        } else {
          QTextStream out(&f);
          if (map->dataset()) {
            for (const auto& [code, li] : map->dataset()->layers.all()) {
              out << QString::fromStdString(li.code) << '\t'
                  << (map->visibility().is_visible(li) ? "on" : "off") << '\n';
            }
          }
          f.close();
          std::fprintf(stdout, "dump-layers saved %s\n", dump_layers.toUtf8().constData());
        }
      }
      QApplication::exit(ok ? 0 : 1);
    });
  }
  return app.exec();
}

