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
  std::vector<Stage *> consideredStages; // only considered stages (stages with at least one range.consider == true), point into `stages`
};

StageMetaInfo getMetaInfoFromStages(std::vector<Stage> &stages);

// A stage is considered if it has at least one range with consider == true
bool isStageConsidered(Stage const &stage);

// Pointers into `stages`, valid while `stages` is not modified
std::vector<Stage *> getConsideredStages(std::vector<Stage> &stages);

float getStagePlaytime(Stage *stage);
int getStageAttempts(Stage *stage);