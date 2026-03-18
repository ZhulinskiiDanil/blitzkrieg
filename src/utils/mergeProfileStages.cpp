#include "mergeProfileStages.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

static bool sameRange(const Range &a, const Range &b)
{
  const float eps = 0.01f;
  return std::fabs(a.from - b.from) < eps && std::fabs(a.to - b.to) < eps;
}

static bool hasMatchingRange(const Stage &oldStage, const Stage &newStage)
{
  for (const auto &rOld : oldStage.ranges)
  {
    for (const auto &rNew : newStage.ranges)
    {
      if (sameRange(rOld, rNew))
        return true;
    }
  }
  return false;
}

std::vector<Stage> mergeProfileStages(
    const std::vector<Stage> &oldStages,
    const std::vector<Stage> &newStages,
    bool automaticallyCloseRuns)
{
  std::vector<Stage> result;

  for (auto stageNew : newStages)
  {
    for (auto &rNew : stageNew.ranges)
    {
      std::vector<std::reference_wrapper<const Range>> allMatches;

      for (const auto &stageOld : oldStages)
      {
        for (const auto &rOld : stageOld.ranges)
        {
          if (sameRange(rOld, rNew))
            allMatches.push_back(rOld);
        }
      }

      if (!allMatches.empty())
      {
        auto itWithProgress = std::find_if(allMatches.begin(), allMatches.end(),
                                           [](const Range &r)
                                           { return r.attempts > 0; });

        if (itWithProgress != allMatches.end())
          rNew = *itWithProgress;
        else
          rNew = allMatches.front();

        rNew.consider = true;
      }
      else
      {
        rNew.consider = true;
        if (automaticallyCloseRuns && stageNew.checked)
        {
          rNew.checked = true;
          rNew.automaticallyClosed = true;
        }
      }
    }

    std::sort(stageNew.ranges.begin(), stageNew.ranges.end(),
              [](const Range &a, const Range &b)
              { return a.from < b.from; });
    stageNew.ranges.erase(std::unique(stageNew.ranges.begin(), stageNew.ranges.end(),
                                      [](const Range &a, const Range &b)
                                      { return sameRange(a, b); }),
                          stageNew.ranges.end());

    result.push_back(stageNew);
  }

  return result;
}