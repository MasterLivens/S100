#include "s100/s100.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cstdio>
#include <functional>
#include <optional>
#include <set>
#include <vector>

#ifndef S100_DEFAULT_FC
#define S100_DEFAULT_FC ""
#endif

namespace {

QPointF project(const QRectF& rect, double west, double south, double east, double north,
                double lon, double lat) {
  const qreal u = static_cast<qreal>((lon - west) / (east - west));
  const qreal v = static_cast<qreal>((north - lat) / (north - south));
  return QPointF(rect.left() + u * rect.width(), rect.top() + v * rect.height());
}

// Placeholder colours keyed by FC code until the Portrayal Catalogue step (contract rule 5).
struct Style {
  QColor fill;  // invalid = outline only
  QColor line;
};

Style style_for(const std::string& code) {
  if (code == "LandArea" || code == "LandRegion") return {QColor(210, 180, 140), QColor(140, 110, 70)};
  if (code == "DepthArea") return {QColor(160, 200, 230), QColor(110, 150, 190)};
  if (code == "DredgedArea") return {QColor(185, 215, 235), QColor(110, 150, 190)};
  if (code == "Coastline" || code == "ShorelineConstruction") return {QColor(), QColor(60, 50, 30)};
  if (code == "DepthContour") return {QColor(), QColor(70, 110, 160)};
  return {QColor(), QColor(90, 90, 90)};
}

class MapWidget : public QWidget {
 public:
  explicit MapWidget(QWidget* parent = nullptr) : QWidget(parent) {
    setMinimumSize(640, 480);
    setMouseTracking(true);
  }

  void setDataset(std::optional<s100::Dataset> ds) {
    dataset_ = std::move(ds);
    if (dataset_) {
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
  s100::LayerVisibility& visibility() { return visibility_; }
  void setStatusCallback(std::function<void(const QString&)> cb) { status_cb_ = std::move(cb); }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(235, 240, 245));
    if (!dataset_) {
      p.setPen(Qt::gray);
      p.drawText(rect(), Qt::AlignCenter, "No dataset — pass a .000 path");
      return;
    }
    const QRectF r = rect();
    // Paint order: areas, then lines, then points and soundings, so labels stay on top.
    std::vector<const s100::Feature*> order;
    order.reserve(dataset_->features.size());
    for (const auto& f : dataset_->features) {
      const s100::LayerInfo* layer = dataset_->layers.find(f.feature_type);
      if (!layer) continue;  // unknown to FC: never drawn (contract rule 3)
      if (!visibility_.is_visible(*layer)) continue;
      order.push_back(&f);
    }
    auto rank = [](s100::GeometryKind k) {
      switch (k) {
        case s100::GeometryKind::Polygon: return 0;
        case s100::GeometryKind::LineString: return 1;
        case s100::GeometryKind::Point: return 2;
        case s100::GeometryKind::MultiPoint: return 3;
        default: return 4;
      }
    };
    std::stable_sort(order.begin(), order.end(),
                     [&](const s100::Feature* a, const s100::Feature* b) {
                       return rank(a->geometry.kind) < rank(b->geometry.kind);
                     });

    for (const s100::Feature* fp : order) {
      const auto& f = *fp;
      const Style st = style_for(f.feature_type);
      switch (f.geometry.kind) {
        case s100::GeometryKind::Polygon: {
          QPainterPath path;
          path.setFillRule(Qt::OddEvenFill);  // interior rings punch holes
          for (const auto& ring : f.geometry.polygon) {
            QPolygonF poly;
            for (const auto& pt : ring.points)
              poly << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
            path.addPolygon(poly);
            path.closeSubpath();
          }
          p.setBrush(st.fill.isValid() ? QBrush(st.fill) : Qt::NoBrush);
          p.setPen(QPen(st.line, st.fill.isValid() ? 0.8 : 0.5));
          p.drawPath(path);
          break;
        }
        case s100::GeometryKind::LineString: {
          QPolygonF line;
          for (const auto& pt : f.geometry.line)
            line << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
          p.setPen(QPen(st.line, 1.2));
          p.setBrush(Qt::NoBrush);
          p.drawPolyline(line);
          break;
        }
        case s100::GeometryKind::Point: {
          const auto s =
              project(r, west_, south_, east_, north_, f.geometry.point.lon, f.geometry.point.lat);
          p.setPen(QPen(QColor(120, 40, 120), 1));
          p.setBrush(QColor(200, 120, 200));
          p.drawEllipse(s, 2.5, 2.5);
          break;
        }
        case s100::GeometryKind::MultiPoint: {
          // Soundings: depth in metres, one decimal under 31 m (simplified S-52 style).
          QFont font = p.font();
          font.setPointSizeF(7.0);
          p.setFont(font);
          p.setPen(QColor(40, 40, 40));
          for (const auto& s : f.geometry.multipoint) {
            const auto pos = project(r, west_, south_, east_, north_, s.lon, s.lat);
            const QString label = s.z < 31.0 ? QString::number(s.z, 'f', 1)
                                             : QString::number(static_cast<int>(s.z));
            p.drawText(QRectF(pos.x() - 20, pos.y() - 8, 40, 16), Qt::AlignCenter, label);
          }
          break;
        }
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
  std::optional<s100::Dataset> dataset_;
  s100::LayerVisibility visibility_;  // session state, not in Dataset (contract rule 2)
  double west_{-0.05}, south_{-0.05}, east_{0.05}, north_{0.05};
  std::function<void(const QString&)> status_cb_;
};

}  // namespace

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  QString cell, snapshot, fc_path = S100_DEFAULT_FC;
  for (int i = 1; i < argc; ++i) {
    const QString a = argv[i];
    if (a == "--snapshot" && i + 1 < argc) snapshot = argv[++i];
    else if (a == "--fc" && i + 1 < argc) fc_path = argv[++i];
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
  meta->setChecked(false);
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

  // FC is product-level: load once per session, share across datasets.
  s100::FeatureCatalogue fc;
  std::string fc_err;
  const bool have_fc = !fc_path.isEmpty() &&
                       s100::load_feature_catalogue(fc_path.toStdString(), fc, &fc_err);

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
      if (map->dataset())  // a use-type switch resets per-layer overrides of that type
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
  } else if (!cell.isEmpty()) {
    s100::Dataset ds;
    s100::LoadError err;
    if (s100::open_dataset(cell.toStdString(), ds, &err)) {
      s100::ClassifyResult cr;
      const bool ok = s100::classify_layers(ds, fc, &cr);
      QString msg = QString("loaded %1 — %2 features, %3 layers (FC %4 %5)")
                        .arg(cell)
                        .arg(static_cast<qulonglong>(ds.stats().feature_count))
                        .arg(static_cast<qulonglong>(ds.layers.size()))
                        .arg(QString::fromStdString(fc.product))
                        .arg(QString::fromStdString(fc.version));
      if (!ok) msg += QString(" — WARNING: %1 codes not in FC, not drawn")
                         .arg(static_cast<qulonglong>(cr.unknown_codes.size()));
      win.statusBar()->showMessage(msg);
      map->setDataset(std::move(ds));
      refresh_list();
    } else {
      win.statusBar()->showMessage(QString::fromStdString(err.message));
    }
  }

  win.resize(1280, 800);
  win.show();

  // Headless smoke hook: s100_viewer <cell> --snapshot out.png  (use QT_QPA_PLATFORM=offscreen)
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
