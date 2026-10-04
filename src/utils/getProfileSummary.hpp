#pragma once

#include <algorithm>

#include "../serialization/profile/index.hpp"

// Aggregated progress of a profile, used by the profiles list.
// Uses the same definitions as getMetaInfoFromStages, but allocates nothing.
struct ProfileSummary
{
  int totalStages = 0;     // stages with at least one considered range
  int completedStages = 0; // considered stages that are deep checked
  int attempts = 0;        // sum over considered ranges of all stages
  float timePlayed = 0.f;  // sum over considered ranges of all stages

  bool isCompleted() const
  {
    return totalStages > 0 && completedStages >= totalStages;
  }

  // 1-based number of the stage being played, matches the Stage Browser title
  int currentStage() const
  {
    return std::min(completedStages + 1, totalStages);
  }

  float progress() const
  {
    return totalStages > 0
               ? static_cast<float>(completedStages) / static_cast<float>(totalStages)
               : 0.f;
  }
};

ProfileSummary getProfileSummary(Profile const &profile);
