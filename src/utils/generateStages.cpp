#include "generateStages.hpp"
#include <Geode/Geode.hpp>
#include <unordered_set>
#include <algorithm>
#include <string>

using namespace geode::prelude;

matjson::Value generateStages(std::vector<float> sps)
{
  // Guarantee the presence of 0 and 100
  if (sps.empty() || sps.front() != 0.f)
  {
    sps.insert(sps.begin(), 0.f);
  }
  if (sps.back() != 100.f)
  {
    sps.push_back(100.f);
  }

  std::vector<Stage> stages;

  // To prevent duplicate ranges
  std::unordered_set<std::string> seen;

  for (size_t i = 0; i < sps.size(); ++i)
  {
    std::vector<Range> stageRanges;

    float from1 = sps[sps.size() - 1 - i];
    float to1 = 100.f;
    std::string key1 = std::to_string(from1) + "-" + std::to_string(to1);

    if (from1 != to1 && !seen.count(key1))
    {
      seen.insert(key1);
      stageRanges.push_back(Range{
          .id = generateUUID(),
          .from = from1,
          .to = to1,
          .checked = false,
          .attempts = 0,
          .note = "",
          .completedAt = 0,
          .completionCounter = 0});
    }

    for (int j = static_cast<int>(sps.size() - i - 1); j > 0; --j)
    {
      float from2 = sps[j - 1];
      float to2 = sps[j + i];
      std::string key2 = std::to_string(from2) + "-" + std::to_string(to2);

      if (from2 != to2 && !seen.count(key2))
      {
        seen.insert(key2);
        stageRanges.push_back(Range{
            .id = generateUUID(),
            .from = from2,
            .to = to2,
            .checked = false,
            .attempts = 0,
            .note = "",
            .completedAt = 0,
            .completionCounter = 0});
      }
    }

    if (!stageRanges.empty())
    {
      std::sort(stageRanges.begin(), stageRanges.end(),
                [](const Range &a, const Range &b)
                { return a.from < b.from; });

      stages.push_back(Stage{
          .id = generateUUID(),
          .stage = static_cast<int>(i + 1),
          .checked = false,
          .note = "",
          .attempts = 0,
          .completionCounter = 0,
          .ranges = stageRanges});
    }
  }

  return matjson::Serialize<std::vector<Stage>>::toJson(stages);
}
