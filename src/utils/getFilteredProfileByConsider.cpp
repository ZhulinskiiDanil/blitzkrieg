#include "./getFilteredProfileByConsider.hpp"

Profile getFilteredProfileByConsider(const Profile &profile)
{
  // Return profile with filtered profile.data.stages
  // Remove stage.ranges that are range.consider == false
  // Remove stage if he's empty after filtering

  Profile filteredProfile = profile;
  std::vector<Stage> filteredStages;

  for (const auto &stage : profile.data.stages)
  {
    Stage filteredStage = stage;
    std::vector<Range> filteredRanges;

    for (const auto &range : stage.ranges)
    {
      if (range.consider)
        filteredRanges.push_back(range);
    }

    if (!filteredRanges.empty())
    {
      filteredStage.ranges = filteredRanges;
      filteredStages.push_back(filteredStage);
    }
  }

  filteredProfile.data.stages = filteredStages;
  return filteredProfile;
}