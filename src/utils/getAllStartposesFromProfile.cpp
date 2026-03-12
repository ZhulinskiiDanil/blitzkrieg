#include "./getAllStartposesFromProfile.hpp"

std::vector<float> getAllStartposesFromProfile(const Profile &profile)
{
  std::vector<float> startposes;
  Stage firstStage = profile.data.stages[0];

  if (firstStage.ranges.empty())
    return startposes;

  for (const auto &range : firstStage.ranges)
    if (range.from > 0 && range.from < 100)
      startposes.push_back(range.from);

  std::sort(startposes.begin(), startposes.end());
  startposes.erase(std::unique(startposes.begin(), startposes.end()), startposes.end());

  return startposes;
}