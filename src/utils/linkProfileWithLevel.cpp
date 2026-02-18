#include "linkProfileWithLevel.hpp"
#include "./unlinkProfileFromLevel.hpp"
#include "../store/GlobalStore.hpp"

void linkProfileWithLevel(const Profile &profile, GJGameLevel *level)
{
  if (!level)
    return;

  std::string levelId = geode::utils::numToString(EditorIDs::getID(level));

  // Unlink all profile(s) from current level
  for (const auto &p : GlobalStore::get()->getProfiles())
  {
    unlinkProfileFromLevel(p, levelId);
  }

  // Link new profile
  std::string key = levelId + "-" + profile.id;
  matjson::Value profileJson = profile;
  Mod::get()->setSavedValue(key, profileJson.dump(matjson::NO_INDENTATION));
}
