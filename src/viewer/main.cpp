#include <algorithm>
#include <vector>
#include "s100/s100.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStatusBar>
#include <QTimer>
#include <cstdio>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>
#include <optional>

namespace {

QPointF project(const QRectF& rect, double west, double south, double east, double north,
                double lon, double lat) {
  const qreal u = static_cast<qreal>((lon - west) / (east - west));
  const qreal v = static_cast<qreal>((north - lat) / (north - south));
  return QPointF(rect.left() + u * rect.width(), rect.top() + v * rect.height());
}

class MapWidget : public QWidget {
 public:
  explicit MapWidget(QWidget* parent = nullptr) : QWidget(parent) {
    setMinimumSize(640, 480);
    setMouseTracking(true);
    layers_.set(s100::LayerId::LAND, true);
    layers_.set(s100::LayerId::DEPARE, true);
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

  s100::LayerVisibility& layers() { return layers_; }

  void setStatusCallback(std::function<void(const QString&)> cb) { status_cb_ = std::move(cb); }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.fillRect(rect(), QColor(20, 40, 60));
    if (!dataset_) {
      p.setPen(Qt::gray);
      p.drawText(rect(), Qt::AlignCenter, "No dataset — pass a .000 path");
      return;
    }
    const QRectF r = rect();
    // Paint order: areas, then lines, then points and soundings, so labels stay on top.
    std::vector<const s100::Feature*> order;
    order.reserve(dataset_->features.size());
    for (const auto& f : dataset_->features) order.push_back(&f);
    auto rank = [](s100::GeometryKind k) {
      switch (k) {
        case s100::GeometryKind::Polygon: return 0;
        case s100::GeometryKind::LineString: return 1;
        case s100::GeometryKind::Point: return 2;
        case s100::GeometryKind::MultiPoint: return 3;
        default: return 4;
      }
    };
    std::stable_sort(order.begin(), order.end(), [&](const s100::Feature* a, const s100::Feature* b) {
      return rank(a->geometry.kind) < rank(b->geometry.kind);
    });
    for (const s100::Feature* fp : order) {
      const auto& f = *fp;
      if (!layers_.is_visible(f.layer)) continue;
      QColor color = f.layer == s100::LayerId::LAND ? QColor(210, 180, 140)
                                                    : QColor(160, 200, 230);
      if (f.geometry.kind == s100::GeometryKind::Empty) continue;
      if (f.geometry.kind == s100::GeometryKind::LineString) {
        QPolygonF poly;
        QVector<QPointF> pts;
        for (const auto& pt : f.geometry.line)
          pts << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
        if (pts.size() >= 2) {
          p.setPen(QPen(color, 1.5));
          for (int i = 1; i < pts.size(); ++i) p.drawLine(pts[i-1], pts[i]);
        }
        continue;
      }
      if (f.geometry.kind == s100::GeometryKind::MultiPoint) {
        // Soundings: depth label in metres, one decimal under 31 m (simplified S-52 style).
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
        continue;
      }
      if (f.geometry.kind == s100::GeometryKind::Point) {
        auto s = project(r, west_, south_, east_, north_, f.geometry.point.lon, f.geometry.point.lat);
        p.setBrush(color); p.drawEllipse(s, 3, 3);
        continue;
      }
      if (f.geometry.kind != s100::GeometryKind::Polygon) continue;
      // Exterior + interior rings in one path; odd-even fill punches holes.
      QPainterPath path;
      path.setFillRule(Qt::OddEvenFill);
      for (const auto& ring : f.geometry.polygon) {
        QPolygonF poly;
        for (const auto& pt : ring.points)
          poly << project(r, west_, south_, east_, north_, pt.lon, pt.lat);
        path.addPolygon(poly);
        path.closeSubpath();
      }
      p.setBrush(color);
      p.setPen(QPen(color.darker(), 1));
      p.drawPath(path);
    }
  }

  void mousePressEvent(QMouseEvent* e) override {
    const auto pick =
        s100::pick_wgs84(static_cast<float>(e->position().x()),
                         static_cast<float>(e->position().y()),
                         static_cast<float>(width()), static_cast<float>(height()), west_,
                         south_, east_, north_);
    if (status_cb_) {
      status_cb_(QString("pick WGS84 lon=%1 lat=%2")
                     .arg(pick.lon, 0, 'f', 8)
                     .arg(pick.lat, 0, 'f', 8));
    }
  }

 private:
  std::optional<s100::Dataset> dataset_;
  s100::LayerVisibility layers_;
  double west_{-0.05}, south_{-0.05}, east_{0.05}, north_{0.05};
  std::function<void(const QString&)> status_cb_;
};

}  // namespace

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QMainWindow win;
  win.setWindowTitle("s100-viewer (P0 C++)");
  auto* central = new QWidget;
  auto* layout = new QVBoxLayout(central);
  auto* bar = new QHBoxLayout;
  auto* land = new QCheckBox("LAND");
  land->setChecked(true);
  auto* depare = new QCheckBox("DEPARE");
  depare->setChecked(true);
  bar->addWidget(land);
  bar->addWidget(depare);
  bar->addStretch();
  auto* map = new MapWidget;
  layout->addLayout(bar);
  layout->addWidget(map, 1);
  win.setCentralWidget(central);
  win.statusBar()->showMessage("Ready");
  map->setStatusCallback([&](const QString& s) { win.statusBar()->showMessage(s); });

  QObject::connect(land, &QCheckBox::toggled, map, [map](bool on) {
    map->layers().set(s100::LayerId::LAND, on);
    map->update();
  });
  QObject::connect(depare, &QCheckBox::toggled, map, [map](bool on) {
    map->layers().set(s100::LayerId::DEPARE, on);
    map->update();
  });

  if (argc >= 2) {
    s100::Dataset ds;
    s100::LoadError err;
    if (s100::open_dataset(argv[1], ds, &err)) {
      win.statusBar()->showMessage(
          QString("loaded %1 — %2 features")
              .arg(argv[1])
              .arg(static_cast<qulonglong>(ds.stats().feature_count)));
      map->setDataset(std::move(ds));
    } else {
      win.statusBar()->showMessage(QString::fromStdString(err.message));
    }
  }

  win.resize(1280, 800);
  win.show();

  // Headless smoke hook: s100_viewer <cell> --snapshot out.png  (use QT_QPA_PLATFORM=offscreen)
  for (int i = 1; i + 1 < argc; ++i) {
    if (QString(argv[i]) == "--snapshot") {
      const QString out = argv[i + 1];
      QTimer::singleShot(200, [&win, out]() {
        const bool ok = win.grab().save(out);
        std::fprintf(ok ? stdout : stderr, "snapshot %s %s\n", ok ? "saved" : "FAILED",
                     out.toUtf8().constData());
        QApplication::exit(ok ? 0 : 1);
      });
    }
  }
  return app.exec();
}
