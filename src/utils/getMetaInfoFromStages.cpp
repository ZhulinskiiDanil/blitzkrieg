#include "./getMetaInfoFromStages.hpp"
#include "./getFirstUncheckedStage.hpp"

StageMetaInfo getMetaInfoFromStages(std::vector<Stage> &stages)
{
  int total = 0;
  int completed = 0;
  float currStagePlaytime = 0;
  int currStageAttempts = 0;
  int currStageTotalRanges = 0;
  int currStageCompletedRanges = 0;
  auto currentStage = getFirstUncheckedStage(stages);
  std::vector<Stage> *consideredStages = new std::vector<Stage>();

  if (currentStage)
  {
    for (auto &range : currentStage->ranges)
    {
      if (range.consider)
      {
        if (range.checked)
          currStageCompletedRanges++;

        currStageTotalRanges++;
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

    if (stageConsidered)
      consideredStages->push_back(stage);

    // A stage is completed if it is considered and has all ranges deep checked
    if (stageConsidered)
    {
      total++;

      if (isStageDeepChecked(stage))
        completed++;
    }
  }

  return {
      total,
      completed,
      currStagePlaytime,
      currStageAttempts,
      currStageTotalRanges,
      currStageCompletedRanges,
      currentStage,
      consideredStages};
}

float getStagePlaytime(Stage *stage)
{
  float res = 0;

  for (auto &range : stage->ranges)
    if (range.consider)
      res += range.timePlayed;

  return res;
}

int getStageAttempts(Stage *stage)
{
  int res = 0;

  for (auto &range : stage->ranges)
    if (range.consider)
      res += range.attempts;

  return res;
}