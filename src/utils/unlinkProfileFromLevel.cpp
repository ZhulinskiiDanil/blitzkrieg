#include "unlinkProfileFromLevel.hpp"

void unlinkProfileFromLevel(const Profile &profile, const std::string &levelId)
{
  std::string key = levelId + "-" + profile.id;
  Mod::get()->setSavedValue<std::string>(key, "");
}

void unlinkProfileFromLevel(const Profile &profile, GJGameLevel *level)
{
  if (!level)
    return;

  std::string levelId = geode::utils::numToString(EditorIDs::getID(level));

  std::string key = levelId + "-" + profile.id;
  Mod::get()->setSavedValue<std::string>(key, "");
}
