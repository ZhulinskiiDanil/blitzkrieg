#include "./getFirstUncheckedStage.hpp"
#include "./isStageDeepChecked.hpp"

Stage *getFirstUncheckedStage(Profile &profile)
{
  for (auto &stage : profile.data.stages)
    if (!isStageDeepChecked(stage))
      return &stage;

  return nullptr;
}

// Range *getFirstUncheckedRange(ProfileData &data)
// {
//   for (auto &stage : data.stages)
//   {
//     for (auto &range : stage.ranges)
//     {
//       if (!range.checked)
//       {
//         return &range;
//       }
//     }
//   }
//   return nullptr;
// }