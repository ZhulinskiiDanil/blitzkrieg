#include "linkProfileWithLevel.hpp"
#include "./unlinkProfileFromLevel.hpp"
#include "../store/GlobalStore.hpp"

void linkProfileWithLevel(const Profile &profile, GJGameLevel *level)
{
  if (!level)
    return;

  std::string levelId = level->m_levelID ? utils::numToString(level->m_levelID.value()) : utils::numToString(EditorIDs::getID(level));

  // Unlink all profile(s) from current level
  for (const auto &p : GlobalStore::get()->getProfiles())
  {
    unlinkProfileFromLevel(p, levelId);
  }

  // Link new profile
  std::string key = levelId + "-" + profile.id;
  Mod::get()->setSavedValue(key, profile.id);
}
