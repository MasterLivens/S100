#include "s100/s100.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QFile>
#include <QHBoxLayout>
#include <QHash>
#include <QImage>
#include <QListWidget>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QSvgRenderer>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <functional>
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

namespace {

namespace fs = std::filesystem;

QPointF project(const QRectF& rect, double west, double south, double east, double north,
                double lon, double lat) {
  const qreal u = static_cast<qreal>((lon - west) / (east - west));
  const qreal v = static_cast<qreal>((north - lat) / (north - south));
  return QPointF(rect.left() + u * rect.width(), rect.top() + v * rect.height());
}

/** Day (or named) palette from PC ColorProfiles/colorProfile.xml — sRGB only. */
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
      missing[ref] = true;
      return nullptr;
    }
    const QByteArray remapped = remap_symbol_svg(f.readAll(), *palette);
    auto renderer = std::make_unique<QSvgRenderer>();
    if (!renderer->load(remapped) || !renderer->isValid()) {
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


QPointF feature_anchor(const s100::Feature& f, const QRectF& r, double west, double south,
                       double east, double north) {
  switch (f.geometry.kind) {
    case s100::GeometryKind::Point:
      return project(r, west, south, east, north, f.geometry.point.lon, f.geometry.point.lat);
    case s100::GeometryKind::MultiPoint:
      if (!f.geometry.multipoint.empty()) {
        const auto& p = f.geometry.multipoint.front();
        return project(r, west, south, east, north, p.lon, p.lat);
      }
      break;
    case s100::GeometryKind::LineString:
      if (!f.geometry.line.empty()) {
        const auto& p = f.geometry.line[f.geometry.line.size() / 2];
        return project(r, west, south, east, north, p.lon, p.lat);
      }
      break;
    case s100::GeometryKind::Polygon:
      if (!f.geometry.polygon.empty() && !f.geometry.polygon.front().points.empty()) {
        double lon = 0, lat = 0;
        const auto& pts = f.geometry.polygon.front().points;
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

class MapWidget : public QWidget {
 public:
  explicit MapWidget(QWidget* parent = nullptr) : QWidget(parent) {
    setMinimumSize(640, 480);
    setMouseTracking(true);
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

  const std::optional<s100::Dataset>& dataset() const { return dataset_; }
  const s100::PortrayalStats& stats() const { return stats_; }
  s100::LayerVisibility& visibility() { return visibility_; }
  void setStatusCallback(std::function<void(const QString&)> cb) { status_cb_ = std::move(cb); }

 protected:
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
    std::vector<const s100::DrawingInstruction*> order;
    order.reserve(instructions_.size());
    for (const auto& di : instructions_) {
      if (di.kind == s100::DrawingInstruction::Kind::Null) continue;
      order.push_back(&di);
    }
    auto plane_order = [&](const std::string& plane) -> int {
      if (!pc_) return plane == "OverRadar" ? 1 : 0;
      const auto it = pc_->display_planes.find(plane);
      return it == pc_->display_planes.end() ? 0 : it->second;
    };
    std::stable_sort(order.begin(), order.end(),
                     [&](const s100::DrawingInstruction* a, const s100::DrawingInstruction* b) {
                       const int pa = plane_order(a->display_plane);
                       const int pb = plane_order(b->display_plane);
                       if (pa != pb) return pa < pb;
                       return a->drawing_priority < b->drawing_priority;
                     });

    for (const s100::DrawingInstruction* dip : order) {
      const auto& di = *dip;
      const auto fit = by_id_.find(di.feature_id);
      if (fit == by_id_.end()) continue;
      const s100::Feature& f = *fit->second;
      const s100::LayerInfo* layer = dataset_->layers.find(f.feature_type);
      if (!layer) continue;  // unknown to FC: never drawn
      if (!visibility_.is_visible(*layer)) continue;

      switch (di.kind) {
        case s100::DrawingInstruction::Kind::Area:
          draw_area(p, r, f, di);
          break;
        case s100::DrawingInstruction::Kind::Line:
          draw_line(p, r, f, di);
          break;
        case s100::DrawingInstruction::Kind::Point:
          draw_point(p, r, f, di);
          break;
        case s100::DrawingInstruction::Kind::Text:
          draw_text(p, r, f, di);
          break;
        default:
          break;
      }
    }
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
  void draw_area(QPainter& p, const QRectF& r, const s100::Feature& f,
                 const s100::DrawingInstruction& di) {
    if (f.geometry.kind != s100::GeometryKind::Polygon) return;
    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);
    for (const auto& ring : f.geometry.polygon) {
      QPolygonF poly;
      for (const auto& pt : ring.points)
        poly << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
      path.addPolygon(poly);
      path.closeSubpath();
    }

    // ColorFill: solid token from Day palette.
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
      if (!f.geometry.polygon.empty() && !f.geometry.polygon.front().points.empty()) {
        const auto& pt = f.geometry.polygon.front().points.front();
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

    if (f.geometry.kind == s100::GeometryKind::LineString) {
      QPolygonF line;
      for (const auto& pt : f.geometry.line)
        line << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
      p.drawPolyline(line);
    } else if (f.geometry.kind == s100::GeometryKind::Polygon) {
      // Area boundary (common for LineInstruction on surface features).
      for (const auto& ring : f.geometry.polygon) {
        QPolygonF poly;
        for (const auto& pt : ring.points)
          poly << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
        if (!poly.isEmpty()) poly << poly.front();
        p.drawPolyline(poly);
      }
    }
  }

  void draw_point(QPainter& p, const QRectF& r, const s100::Feature& f,
                  const s100::DrawingInstruction& di) {
    // PointInstruction: PC Symbols/<symbol_ref>.svg via QSvgRenderer. CSS class tokens
    // (fCHBLK / sSNDG1 / …) are remapped through the active Day colour profile at load.
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

    if (f.geometry.kind == s100::GeometryKind::Point) {
      paint_one(project(r, west_, south_, east_, north_, f.geometry.point.lon, f.geometry.point.lat));
    } else if (f.geometry.kind == s100::GeometryKind::MultiPoint) {
      for (const auto& pt : f.geometry.multipoint)
        paint_one(project(r, west_, south_, east_, north_, pt.lon, pt.lat));
    } else {
      paint_one(feature_anchor(f, r, west_, south_, east_, north_));
    }
  }

  void draw_text(QPainter& p, const QRectF& r, const s100::Feature& f,
                 const s100::DrawingInstruction& di) {
    if (di.text.empty()) return;
    const QColor col = palette_.get_alpha(di.text_style.font_colour,
                                          di.text_style.font_colour_transparency,
                                          palette_.get("CHBLK", Qt::black));
    QFont font = p.font();
    // FontSize is in mm (Part 9a); map to point size roughly.
    const qreal px = mm_to_px(this, di.text_style.font_size > 0 ? di.text_style.font_size : 2.5);
    font.setPixelSize(std::max(8, static_cast<int>(std::lround(px))));
    if (di.text_style.font_weight == "Bold") font.setBold(true);
    if (di.text_style.font_slant == "Italics" || di.text_style.font_slant == "Italic")
      font.setItalic(true);
    p.setFont(font);
    p.setPen(col);

    int align = Qt::AlignCenter;
    if (di.text_style.halign == "Start" || di.text_style.halign == "Left")
      align = (align & ~Qt::AlignHorizontal_Mask) | Qt::AlignLeft;
    else if (di.text_style.halign == "End" || di.text_style.halign == "Right")
      align = (align & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight;

    const QPointF s = feature_anchor(f, r, west_, south_, east_, north_);
    const qreal ox = mm_to_px(this, di.offset_x);
    const qreal oy = -mm_to_px(this, di.offset_y);  // LocalOffset y up in portrayal CRS
    p.drawText(QRectF(s.x() + ox - 40, s.y() + oy - 12, 80, 24), align,
               QString::fromStdString(di.text));
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
  double west_{-0.05}, south_{-0.05}, east_{0.05}, north_{0.05};
  std::function<void(const QString&)> status_cb_;
};

}  // namespace

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  QString cell, snapshot;
  QString fc_path = S100_DEFAULT_FC;
  QString pc_path = S100_DEFAULT_PC;
  for (int i = 1; i < argc; ++i) {
    const QString a = argv[i];
    if (a == "--snapshot" && i + 1 < argc) snapshot = argv[++i];
    else if (a == "--fc" && i + 1 < argc) fc_path = argv[++i];
    else if (a == "--pc" && i + 1 < argc) pc_path = argv[++i];
    else if (cell.isEmpty()) cell = a;
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

  // FC / PC are product-level: load once per session. Context lives outside Dataset.
  s100::FeatureCatalogue fc;
  s100::PortrayalCatalogue pc;
  std::string fc_err, pc_err;
  const bool have_fc = !fc_path.isEmpty() &&
                       s100::load_feature_catalogue(fc_path.toStdString(), fc, &fc_err);
  const bool have_pc = !pc_path.isEmpty() &&
                       s100::load_portrayal_catalogue(pc_path.toStdString(), pc, &pc_err);

  ColorPalette palette;
  QString pal_err;
  if (have_pc) {
    fs::path profile = pc.root / "ColorProfiles" / "colorProfile.xml";
    if (!pc.color_profiles.empty() && !pc.color_profiles.front().file_name.empty())
      profile = pc.root / "ColorProfiles" / pc.color_profiles.front().file_name;
    if (!load_palette(profile, "Day", palette, &pal_err))
      std::fprintf(stderr, "colour profile: %s\n", pal_err.toUtf8().constData());
  }

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
    if (s100::open_dataset(cell.toStdString(), ds, &err)) {
      s100::ClassifyResult cr;
      const bool ok = s100::classify_layers(ds, fc, &cr);
      s100::PortrayalSession session(fc, pc);
      s100::PortrayalStats ps;
      std::string perr;
      const auto* di = session.portray(ds, &ps, &perr);
      QString msg;
      if (di) {
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
        map->setPortrayal(std::move(ds), *di, ps, palette, pc.root, &pc);
        refresh_list();
      } else {
        msg = QString("portray failed: %1").arg(QString::fromStdString(perr));
        map->setPortrayal(std::move(ds), {}, ps, palette, pc.root, &pc);
        refresh_list();
      }
      win.statusBar()->showMessage(msg);
    } else {
      win.statusBar()->showMessage(QString::fromStdString(err.message));
    }
  }

  win.resize(1280, 800);
  win.show();

  // Headless smoke: s100_viewer <cell> --snapshot out.png  (QT_QPA_PLATFORM=offscreen)
  if (!snapshot.isEmpty()) {
    QTimer::singleShot(200, [&win, snapshot]() {
      const bool ok = win.grab().save(snapshot);
      std::fprintf(ok ? stdout : stderr, "snapshot %s %s\n", ok ? "saved" : "FAILED",
                   snapshot.toUtf8().constData());
      QApplication::exit(ok ? 0 : 1);
    });
  }
  return app.exec();
}
