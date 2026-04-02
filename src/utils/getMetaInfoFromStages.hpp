#pragma once

#include "../serialization/profile/index.hpp"
#include "./isStageDeepChecked.hpp"

struct StageMetaInfo
{
  int total;                            // amount of stages that has at least one range.consider == true
  int completed;                        // amount of deep checked stages
  float currStagePlaytime;              // total playtime of the current stage (sum of timePlayed of all ranges in the current stage)
  int currStageAttempts;                // total attempts of the current stage (sum of attempts of all ranges in the current stage)
  int currStageTotalRanges;             // total ranges in the current stage
  int currStageCompletedRanges;         // total completed ranges in the current stage
  Stage *currentStage;                  // pointer to the current stage (first stage with at least one range.consider == true and checked == false)
  std::vector<Stage> *consideredStages; // only considered stages (stages with at least one range.consider == true)
};

StageMetaInfo getMetaInfoFromStages(std::vector<Stage> &stages);

float getStagePlaytime(Stage *stage);
int getStageAttempts(Stage *stage);