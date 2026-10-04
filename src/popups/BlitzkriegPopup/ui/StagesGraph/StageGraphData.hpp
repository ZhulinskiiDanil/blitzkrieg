#pragma once
#include <ctime>
#include <optional>
#include <string>
#include <vector>

#include "../../../../serialization/profile/index.hpp"

enum class StageGraphMetric
{
  Attempts,
  Playtime,
  // Not a bar metric, the layer shows the timeline chart instead
  Timeline
};

enum class StageGraphStatus
{
  Completed,
  Current,
  Upcoming
};

// One considered run of a stage
struct StageGraphRun
{
  Range *range = nullptr;

  float from = 0.f;
  float to = 0.f;

  int attempts = 0;
  float timePlayed = 0.f;

  bool checked = false;
  std::time_t completedAt = 0;

  float getValue(StageGraphMetric metric) const
  {
    return metric == StageGraphMetric::Playtime
               ? timePlayed
               : static_cast<float>(attempts);
  }
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

  // Considered runs, bottom to top of the bar
  std::vector<StageGraphRun> runs;

  // When the last run of a completed stage was closed, 0 when unknown
  std::time_t completedAt = 0;

  float getValue(StageGraphMetric metric) const
  {
    return metric == StageGraphMetric::Playtime
               ? timePlayed
               : static_cast<float>(attempts);
  }
};

// Rough estimate of what is left: the average of a completed stage
// for every stage that is not completed, minus what the current one already took
struct StageGraphForecast
{
  float attempts = 0.f;
  float time = 0.f;
  int stagesLeft = 0;

  // Every stage is completed
  bool done = false;
  // False while no stage is completed, there is nothing to average
  bool known = false;
};

std::vector<StageGraphColumn> buildStageGraphColumns(Profile &profile);

// Position of a percentage reached from 0% on the column axis.
// Stage N holds runs of N segments, so its run from 0% ends at `endPercent`,
// and the percentage falls between the two columns around it.
// The result is fractional, 0 is the center of the first column.
std::optional<float> mapPercentFromZero(
    std::vector<StageGraphColumn> const &columns,
    float percent);

StageGraphForecast estimateRemaining(std::vector<StageGraphColumn> const &columns);

// 1, 2 or 5 times a power of 10, at least 1
float getNiceAxisStep(float raw);

// 950, 1.2k, 3M; units are wrapped in <small> for UILabel
std::string formatCompactNumber(float value);
