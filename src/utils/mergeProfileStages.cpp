#include "mergeProfileStages.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

static bool sameRange(const Range &a, const Range &b)
{
  const float eps = 0.001f;
  return std::fabs(a.from - b.from) < eps && std::fabs(a.to - b.to) < eps;
}

// Проверяем, совпадает ли хотя бы один range в старой и новой стадии
static bool hasMatchingRange(const Stage &oldStage, const Stage &newStage)
{
  for (const auto &rOld : oldStage.ranges)
  {
    for (const auto &rNew : newStage.ranges)
    {
      if (sameRange(rOld, rNew))
        return true;
    }
  }
  return false;
}

std::vector<Stage> mergeProfileStages(
    const std::vector<Stage> &oldStages,
    const std::vector<Stage> &newStages,
    bool automaticallyCloseRuns)
{
  std::vector<Stage> result;

  // 1️⃣ Перебираем новые стадии
  for (auto stageNew : newStages)
  {
    // Ищем старую стадию с совпадающим range
    auto itOld = std::find_if(
        oldStages.begin(), oldStages.end(),
        [&](const Stage &sOld)
        { return hasMatchingRange(sOld, stageNew); });

    if (itOld != oldStages.end())
    {
      const Stage &stageOld = *itOld;

      // Перебираем range’ы новой стадии
      for (auto &rNew : stageNew.ranges)
      {
        auto itR = std::find_if(
            stageOld.ranges.begin(), stageOld.ranges.end(),
            [&](const Range &rOld)
            { return sameRange(rOld, rNew); });

        rNew.consider = true;

        // Совпадает → переносим прогресс
        if (itR != stageOld.ranges.end())
        {
          rNew = *itR;
        }
        else
        {
          // Новый range
          if (automaticallyCloseRuns && stageNew.checked)
          {
            rNew.checked = true;
            rNew.automaticallyClosed = true;
          }
        }
      }

      // Добавляем старые range’ы, которых нет в новой стадии
      for (const auto &rOld : stageOld.ranges)
      {
        auto itR = std::find_if(
            stageNew.ranges.begin(), stageNew.ranges.end(),
            [&](const Range &rNew)
            { return sameRange(rOld, rNew); });

        if (itR == stageNew.ranges.end())
        {
          Range hidden = rOld;
          hidden.consider = false;
          stageNew.ranges.push_back(hidden);
        }
      }

      result.push_back(stageNew);
    }
    else
    {
      // Новая стадия → просто включаем
      for (auto &r : stageNew.ranges)
      {
        r.consider = true;
        if (automaticallyCloseRuns && stageNew.checked)
        {
          r.checked = true;
          r.automaticallyClosed = true;
        }
      }
      result.push_back(stageNew);
    }
  }

  // 2️⃣ Добавляем старые стадии, которых нет в новых
  for (const auto &stageOld : oldStages)
  {
    auto it = std::find_if(
        result.begin(), result.end(),
        [&](const Stage &s)
        { return hasMatchingRange(stageOld, s); });

    if (it == result.end())
    {
      Stage hiddenStage = stageOld;
      for (auto &r : hiddenStage.ranges)
        r.consider = false;
      result.push_back(hiddenStage);
    }
  }

  return result;
}