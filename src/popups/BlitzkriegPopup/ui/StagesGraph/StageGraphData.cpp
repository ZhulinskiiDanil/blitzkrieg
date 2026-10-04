#include "StageGraphData.hpp"

#include <algorithm>
#include <cmath>
#include <fmt/core.h>
#include <Geode/Geode.hpp>

#include "../../../../utils/getMetaInfoFromStages.hpp"
#include "../../../../utils/isStageDeepChecked.hpp"

std::vector<StageGraphColumn> buildStageGraphColumns(Profile &profile)
{
  std::vector<StageGraphColumn> columns;
  int progressIndex = -1;

  const auto stages = getConsideredStages(profile.data.stages);

  for (std::size_t i = 0; i < stages.size(); ++i)
  {
    auto *stage = stages[i];

    StageGraphColumn column;
    column.stage = stage;
    column.index = static_cast<int>(i);
    column.attempts = getStageAttempts(stage);
    column.timePlayed = getStagePlaytime(stage);

    for (auto &range : stage->ranges)
    {
      if (std::abs(range.from) < .01f)
        column.endPercent = range.to;

      if (!range.consider)
        continue;

      column.totalRuns++;

      if (range.checked)
        column.completedRuns++;

      column.runs.push_back({
          .range = &range,
          .from = range.from,
          .to = range.to,
          .attempts = range.attempts,
          .timePlayed = range.timePlayed,
          .checked = range.checked,
          .completedAt = range.checked ? range.completedAt : 0,
      });
    }

    std::sort(
        column.runs.begin(),
        column.runs.end(),
        [](StageGraphRun const &a, StageGraphRun const &b)
        { return a.from < b.from; });

    // The first stage that is not completed is the one the player is on
    if (progressIndex < 0 && !isStageDeepChecked(*stage))
    {
      progressIndex = static_cast<int>(i);
      column.status = StageGraphStatus::Current;
    }
    else if (progressIndex < 0)
    {
      column.status = StageGraphStatus::Completed;
    }

    // The stage is done when its last run is closed
    if (isStageDeepChecked(*stage))
    {
      for (auto const &run : column.runs)
        column.completedAt = std::max(column.completedAt, run.completedAt);
    }

    columns.push_back(column);
  }

  return columns;
}

std::optional<float> mapPercentFromZero(
    std::vector<StageGraphColumn> const &columns,
    float percent)
{
  if (percent <= 0.f)
    return std::nullopt;

  // Columns whose run from 0% is known, they go in increasing order
  std::vector<StageGraphColumn const *> known;

  for (auto const &column : columns)
  {
    if (column.endPercent > 0.f)
      known.push_back(&column);
  }

  if (known.empty())
    return std::nullopt;

  auto const *first = known.front();
  auto const *last = known.back();

  // Before the first column: left half of the first slot
  if (percent < first->endPercent)
    return first->index - .5f + .5f * percent / first->endPercent;

  if (percent >= last->endPercent)
    return static_cast<float>(last->index);

  for (std::size_t i = 0; i + 1 < known.size(); ++i)
  {
    auto const *a = known[i];
    auto const *b = known[i + 1];

    if (percent < b->endPercent)
    {
      const float t = (percent - a->endPercent) / (b->endPercent - a->endPercent);
      return a->index + t * (b->index - a->index);
    }
  }

  return static_cast<float>(last->index);
}

StageGraphForecast estimateRemaining(std::vector<StageGraphColumn> const &columns)
{
  StageGraphForecast forecast;

  int completed = 0;
  float completedAttempts = 0.f;
  float completedTime = 0.f;
  // What the stages that are not completed already took
  float spentAttempts = 0.f;
  float spentTime = 0.f;

  for (auto const &column : columns)
  {
    if (column.status == StageGraphStatus::Completed)
    {
      completed++;
      completedAttempts += column.attempts;
      completedTime += column.timePlayed;
      continue;
    }

    forecast.stagesLeft++;
    spentAttempts += column.attempts;
    spentTime += column.timePlayed;
  }

  forecast.done = forecast.stagesLeft == 0 && !columns.empty();
  forecast.known = completed > 0;

  if (!forecast.known || forecast.done)
    return forecast;

  forecast.attempts = std::max(0.f, completedAttempts / completed * forecast.stagesLeft - spentAttempts);
  forecast.time = std::max(0.f, completedTime / completed * forecast.stagesLeft - spentTime);

  return forecast;
}

float getNiceAxisStep(float raw)
{
  if (raw <= 1.f)
    return 1.f;

  const float power = std::pow(10.f, std::floor(std::log10(raw)));
  const float n = raw / power;

  if (n <= 1.f)
    return power;
  if (n <= 2.f)
    return 2.f * power;
  if (n <= 5.f)
    return 5.f * power;

  return 10.f * power;
}

std::string formatCompactNumber(float value)
{
  if (value < 1000.f)
    return fmt::format("{}", static_cast<int>(std::round(value)));

  const bool isMillions = value >= 1000000.f;
  const float scaled = value / (isMillions ? 1000000.f : 1000.f);
  std::string number = fmt::format("{:.1f}", scaled);

  // 2.0k -> 2k
  if (number.size() > 2 && number.compare(number.size() - 2, 2, ".0") == 0)
    number.erase(number.size() - 2);

  return number + (isMillions ? "<small>M</small>" : "<small>k</small>");
}

const char *getOpenHint()
{
#ifdef GEODE_IS_DESKTOP
  return "Click to open";
#else
  return "Tap again to open";
#endif
}
