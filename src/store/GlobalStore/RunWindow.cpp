#include "RunWindow.hpp"

#include <algorithm>
#include <cmath>

float RunWindow::overlap(Range const *range) const
{
  const float overlapStart =
      std::max(start, range->from);

  const float overlapEnd =
      std::min(end, range->to);

  return std::max(
      0.0f,
      overlapEnd - overlapStart);
}

float RunWindow::coverage(Range const *range) const
{
  const float rangeLength =
      range->to - range->from;

  if (rangeLength <= eps)
    return 0.0f;

  return overlap(range) /
         rangeLength;
}

bool RunWindow::touches(Range const *range) const
{
  return overlap(range) > eps;
}

bool RunWindow::passes(Range const *range) const
{
  return start <= range->from + eps &&
         end + eps >= range->to;
}

bool RunWindow::isBetterPassable(
    Range const *candidate,
    Range const *current,
    bool checked) const
{
  if (!current)
    return true;

  const float candidateFromDistance =
      std::abs(
          candidate->from -
          start);

  const float currentFromDistance =
      std::abs(
          current->from -
          start);

  if (
      std::fabs(
          candidateFromDistance -
          currentFromDistance) > eps)
  {
    return candidateFromDistance <
           currentFromDistance;
  }

  if (
      std::fabs(
          candidate->from -
          current->from) > eps)
  {
    return candidate->from <
           current->from;
  }

  if (checked)
  {
    return candidate->to >
           current->to + eps;
  }

  return candidate->to <
         current->to - eps;
}

bool RunWindow::isBetterCoverage(
    Range const *candidate,
    Range const *current,
    bool checked) const
{
  if (!current)
    return true;

  const float candidateCoverage =
      coverage(candidate);

  const float currentCoverage =
      coverage(current);

  if (
      std::fabs(
          candidateCoverage -
          currentCoverage) > eps)
  {
    return candidateCoverage >
           currentCoverage;
  }

  const float candidateFromDistance =
      std::abs(
          candidate->from -
          start);

  const float currentFromDistance =
      std::abs(
          current->from -
          start);

  if (
      std::fabs(
          candidateFromDistance -
          currentFromDistance) > eps)
  {
    return candidateFromDistance <
           currentFromDistance;
  }

  if (
      std::fabs(
          candidate->from -
          current->from) > eps)
  {
    return candidate->from >
           current->from;
  }

  if (checked)
  {
    return candidate->to >
           current->to + eps;
  }

  return candidate->to <
         current->to - eps;
}

Range *RunWindow::selectStatsRange(
    std::vector<Range *> const &candidates,
    bool checked,
    const char *&rule) const
{
  Range *selected = nullptr;

  for (auto *range : candidates)
  {
    if (!passes(range))
      continue;

    if (isBetterPassable(range, selected, checked))
    {
      selected = range;
      rule = "passable/nearest start";
    }
  }

  if (selected)
    return selected;

  for (auto *range : candidates)
  {
    if (isBetterCoverage(range, selected, checked))
    {
      selected = range;
      rule = "best coverage";
    }
  }

  return selected;
}
