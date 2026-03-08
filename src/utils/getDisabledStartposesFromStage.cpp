#include "./getDisabledStartposesFromStage.hpp"

static bool sameRangeByFrom(const Range &a, const Range &b)
{
  const float eps = 0.01f;
  return std::fabs(a.from - b.from) < eps;
}

std::vector<float> getDisabledStartposesFromStage(const Stage &stage)
{
  std::vector<float> startposes;

  if (stage.ranges.empty())
    return startposes;

  for (const auto &range : stage.ranges)
  {
    bool hasConsideredRange = false;

    if (range.from == 0)
      continue;

    for (const auto &other : stage.ranges)
    {
      if (sameRangeByFrom(range, other) && other.consider)
      {
        hasConsideredRange = true;
        break;
      }
    }

    if (!hasConsideredRange)
      startposes.push_back(range.from);
  }

  return startposes;
}