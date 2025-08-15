#include "getProfileStats.hpp"

ProfileStats getProfileStats(const Profile &profile)
{
  ProfileStats stats;
  stats.totalStages = static_cast<int>(profile.data.stages.size());
  stats.currentStage = stats.totalStages; // по умолчанию — последний, если все пройдены

  for (size_t i = 0; i < profile.data.stages.size(); ++i)
  {
    if (!profile.data.stages[i].checked)
    {
      stats.currentStage = static_cast<int>(i + 1); // 1-based
      break;
    }
  }

  return stats;
}