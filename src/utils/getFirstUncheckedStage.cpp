#include "./getFirstUncheckedStage.hpp"
#include "./isStageDeepChecked.hpp"

Stage *getFirstUncheckedStage(Profile &profile)
{
  for (auto &stage : profile.data.stages)
    if (!isStageDeepChecked(stage))
      return &stage;

  return nullptr;
}

Stage *getFirstUncheckedStage(std::vector<Stage> &stages)
{
  for (auto &stage : stages)
    if (!isStageDeepChecked(stage))
      return &stage;

  return nullptr;
}