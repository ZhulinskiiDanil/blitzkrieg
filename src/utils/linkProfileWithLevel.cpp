#include "linkProfileWithLevel.hpp"
#include "./unlinkProfileFromLevel.hpp"
#include "../store/GlobalStore.hpp"

void linkProfileWithLevel(const Profile &profile, GJGameLevel *level)
{
  if (!level || !&profile)
    return;

  std::string lvlId = !level->getID().empty()
                          ? level->getID()
                          : geode::utils::numToString(EditorIDs::getID(level));

  // Unlink all profile(s) from current level
  for (const auto &p : GlobalStore::get()->getProfiles())
  {
    unlinkProfileFromLevel(p, lvlId);
  }

  // Link new profile
  std::string key = lvlId + "-" + profile.profileName;
  matjson::Value profileJson = profile;
  Mod::get()->setSavedValue(key, profileJson.dump(matjson::NO_INDENTATION));
}
