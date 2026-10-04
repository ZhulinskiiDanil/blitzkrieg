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
  std::vector<Stage *> consideredStages = getConsideredStages(stages);

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

  for (auto *stage : consideredStages)
  {
    total++;

    // A stage is completed if it is considered and has all ranges deep checked
    if (isStageDeepChecked(*stage))
      completed++;
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

bool isStageConsidered(Stage const &stage)
{
  for (auto const &range : stage.ranges)
    if (range.consider)
      return true;

  return false;
}

std::vector<Stage *> getConsideredStages(std::vector<Stage> &stages)
{
  std::vector<Stage *> res;

  for (auto &stage : stages)
    if (isStageConsidered(stage))
      res.push_back(&stage);

  return res;
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