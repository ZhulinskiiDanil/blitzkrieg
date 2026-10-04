#pragma once
#include <optional>
#include <string>
#include <vector>

#include "../../../../serialization/profile/index.hpp"

enum class StageGraphMetric
{
  Attempts,
  Playtime
};

enum class StageGraphStatus
{
  Completed,
  Current,
  Upcoming
};

// One column of the Stage Graph, a considered stage of the profile
struct StageGraphColumn
{
  Stage *stage = nullptr;
  // Index among considered stages, the same scale as the Stage Browser
  int index = 0;

  int attempts = 0;
  float timePlayed = 0.f;

  int completedRuns = 0;
  int totalRuns = 0;

  StageGraphStatus status = StageGraphStatus::Upcoming;

  // `to` of the run that starts at 0%, -1 when the stage has none
  float endPercent = -1.f;

  float getValue(StageGraphMetric metric) const
  {
    return metric == StageGraphMetric::Attempts
               ? static_cast<float>(attempts)
               : timePlayed;
  }
};

std::vector<StageGraphColumn> buildStageGraphColumns(Profile &profile);

// Position of a percentage reached from 0% on the column axis.
// Stage N holds runs of N segments, so its run from 0% ends at `endPercent`,
// and the percentage falls between the two columns around it.
// The result is fractional, 0 is the center of the first column.
std::optional<float> mapPercentFromZero(
    std::vector<StageGraphColumn> const &columns,
    float percent);

// 950, 1.2k, 3M; units are wrapped in <small> for UILabel
std::string formatCompactNumber(float value);
