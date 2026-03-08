#include "./getMetaInfoFromStages.hpp"
#include "./getFirstUncheckedStage.hpp"

MetaInfo getMetaInfoFromStages(std::vector<Stage> &stages)
{
  int total = 0;
  int completed = 0;
  auto currentStage = getFirstUncheckedStage(stages);
  int currStagePlaytime = 0;
  int currStageAttempts = 0;

  if (currentStage)
  {
    for (auto &range : currentStage->ranges)
    {
      if (range.consider)
      {
        currStagePlaytime += range.timePlayed;
        currStageAttempts += range.attempts;
      }
    }
  }

  for (auto &stage : stages)
  {
    bool stageConsidered = false;

    // A stage is considered if it has at least one range with consider == true
    for (auto &range : stage.ranges)
      if (range.consider)
        stageConsidered = true;

    // A stage is completed if it is considered and has all ranges deep checked
    if (stageConsidered)
    {
      total++;

      if (isStageDeepChecked(stage))
        completed++;
    }
  }

  return {total, completed, currentStage, currStagePlaytime, currStageAttempts};
}