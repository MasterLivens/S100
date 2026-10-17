#pragma once
// S-104 / S-111 timeline list + step helpers (docs/timeline-ui-contract.md).
#include "s100/current.hpp"
#include "s100/water_level.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace s100 {

struct TimelineEntry {
  std::size_t index{0};
  std::string time_iso;  // may be empty if dataset omitted time
  bool active{false};
};

/** Fill entries from wl.frames; mark active_frame. Returns false if frames empty. */
bool timeline_entries_water_level(const WaterLevelDataset& wl, std::vector<TimelineEntry>& out);

/** Fill entries from cur.frames; mark active_frame. Returns false if frames empty. */
bool timeline_entries_current(const CurrentDataset& cur, std::vector<TimelineEntry>& out);

/** +1/-1 clamp at 0 or size-1; true iff active_frame changed; false if empty. */
bool timeline_step_frame(WaterLevelDataset& wl, int delta);
bool timeline_step_frame(CurrentDataset& cur, int delta);

}  // namespace s100
