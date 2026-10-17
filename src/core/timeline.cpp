#include "s100/timeline.hpp"


namespace s100 {
namespace {

template <typename Dataset>
bool fill_entries(const Dataset& ds, std::vector<TimelineEntry>& out) {
  out.clear();
  if (ds.frames.empty()) return false;
  out.reserve(ds.frames.size());
  for (std::size_t i = 0; i < ds.frames.size(); ++i) {
    TimelineEntry e;
    e.index = i;
    e.time_iso = ds.frames[i].time_iso;
    e.active = (i == ds.active_frame);
    out.push_back(std::move(e));
  }
  return true;
}

template <typename Dataset>
bool step_frame(Dataset& ds, int delta) {
  if (ds.frames.empty()) return false;
  const std::size_t n = ds.frames.size();
  const std::size_t cur = ds.active_frame;
  std::size_t next = cur;
  if (delta > 0) {
    if (cur + 1 < n) next = cur + 1;
  } else if (delta < 0) {
    if (cur > 0) next = cur - 1;
  } else {
    return false;
  }
  if (next == cur) return false;
  return ds.set_active_frame(next);
}

}  // namespace

bool timeline_entries_water_level(const WaterLevelDataset& wl, std::vector<TimelineEntry>& out) {
  return fill_entries(wl, out);
}

bool timeline_entries_current(const CurrentDataset& cur, std::vector<TimelineEntry>& out) {
  return fill_entries(cur, out);
}

bool timeline_step_frame(WaterLevelDataset& wl, int delta) { return step_frame(wl, delta); }

bool timeline_step_frame(CurrentDataset& cur, int delta) { return step_frame(cur, delta); }

}  // namespace s100
