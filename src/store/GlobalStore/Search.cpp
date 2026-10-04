#include "../GlobalStore.hpp"

using namespace geode::prelude;

Profile *GlobalStore::getProfileById(const std::string &profileId)
{
  auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                         [&](const Profile &p)
                         {
                           return p.id == profileId;
                         });

  if (it != m_profiles.end())
  {
    return &(*it);
  }

  return nullptr;
}

Profile *GlobalStore::getProfileByLevel(GJGameLevel *level)
{
  if (!level)
    return {};

  std::string levelId = level->m_levelID ? utils::numToString(level->m_levelID.value()) : utils::numToString(EditorIDs::getID(level));
  return getProfileByLevel(levelId);
}

Profile *GlobalStore::getProfileByLevel(std::string const &levelId)
{
  for (auto &profile : m_profiles)
  {
    std::string key = levelId + "-" + profile.id;
    auto savedStr = Mod::get()->getSavedValue<std::string>(key);

    if (!savedStr.empty())
      return &profile;
  }

  return nullptr;
}

Range GlobalStore::getCurrentRange(std::string const &profileId)
{
  const float eps = 0.01f;
  Range *maxRange = nullptr;
  int currentStage = 0;

  for (auto &profile : m_profiles)
  {
    if (profile.id != profileId)
      continue;

    for (auto &stage : profile.data.stages)
    {
      if (stage.checked)
        continue;

      if (currentStage == 0)
        currentStage = stage.stage;
      else
        break;

      for (auto &range : stage.ranges)
      {
        if (range.consider && !range.checked && std::abs(range.from - runStart) < eps)
        {
          if (!maxRange || range.from > maxRange->from)
          {
            maxRange = &range;
          }
        }
      }
    }
  }

  if (maxRange)
    return *maxRange;

  return {};
}
