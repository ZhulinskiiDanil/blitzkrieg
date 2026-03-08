#pragma once

#include "../serialization/profile/index.hpp"
#include "./isStageDeepChecked.hpp"

struct MetaInfo
{
  int total;             // amount of stages that has at least one range.consider == true
  int completed;         // amount of deep checked stages
  Stage *currentStage;   // pointer to the current stage (first stage with at least one range.consider == true and checked == false)
  int currStagePlaytime; // total playtime of the current stage (sum of timePlayed of all ranges in the current stage)
  int currStageAttempts; // total attempts of the current stage (sum of attempts of all ranges in the current stage)
};

MetaInfo getMetaInfoFromStages(std::vector<Stage> &stages);
