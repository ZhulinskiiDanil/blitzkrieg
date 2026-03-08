#include "./isStageDeepChecked.hpp"

/**
 * Check if all considered ranges in the stage are checked
 */
bool isStageDeepChecked(const Stage &stage)
{
  for (const auto &range : stage.ranges)
  {
    if (!range.checked && range.consider)
      return false;
  }

  return true;
}