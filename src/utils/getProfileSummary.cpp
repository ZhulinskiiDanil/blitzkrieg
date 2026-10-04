#include "./getProfileSummary.hpp"
#include "./isStageDeepChecked.hpp"

ProfileSummary getProfileSummary(Profile const &profile)
{
  ProfileSummary summary;

  for (auto const &stage : profile.data.stages)
  {
    bool considered = false;

    for (auto const &range : stage.ranges)
    {
      if (!range.consider)
        continue;

      considered = true;
      summary.attempts += range.attempts;
      summary.timePlayed += range.timePlayed;
    }

    if (!considered)
      continue;

    summary.totalStages++;

    if (isStageDeepChecked(stage))
      summary.completedStages++;
  }

  return summary;
}
