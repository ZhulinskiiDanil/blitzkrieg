#pragma once

#include <vector>

#include "../../serialization/profile/index.hpp"

// Geometry of a single attempt [start, end] against stage ranges.
// Used by GlobalStore::checkRun to decide which range an attempt belongs to.
struct RunWindow
{
  static constexpr float eps = 0.01f;

  float start = 0.f;
  float end = 0.f;

  float overlap(Range const *range) const;
  float coverage(Range const *range) const;

  bool touches(Range const *range) const;
  bool passes(Range const *range) const;

  bool isBetterPassable(
      Range const *candidate,
      Range const *current,
      bool checked) const;

  bool isBetterCoverage(
      Range const *candidate,
      Range const *current,
      bool checked) const;

  // Picks the range that receives this attempt's stats.
  // Passable ranges win first; otherwise the best covered one is used.
  // `rule` receives a short description of the rule that matched.
  Range *selectStatsRange(
      std::vector<Range *> const &candidates,
      bool checked,
      const char *&rule) const;
};
