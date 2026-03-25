#include "./getFirstUncheckedStage.hpp"
#include "./isStageDeepChecked.hpp"

Stage *getFirstUncheckedStage(Profile &profile)
{
  if (profile.id.empty())
    return nullptr;

  for (auto &stage : profile.data.stages)
    if (!isStageDeepChecked(stage))
      return &stage;

  return nullptr;
}

Stage *getFirstUncheckedStage(std::vector<Stage> &stages)
{
  if (stages.empty())
    return nullptr;

  for (size_t i = 0; i < stages.size(); i++)
  {
    Stage *stage = &stages[i];

    if (!stage)
      continue;

    if (!isStageDeepChecked(*stage))
      return stage;
  }

  return nullptr;
}