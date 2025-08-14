#include "unlinkProfileFromLevel.hpp"

void unlinkProfileFromLevel(const Profile &profile, const std::string &levelId)
{
  std::string key = levelId + "-" + profile.profileName;
  Mod::get()->setSavedValue<std::string>(key, "");
}

void unlinkProfileFromLevel(const Profile &profile, GJGameLevel *level)
{
  if (!level)
    return;

  std::string levelId = !level->getID().empty()
                            ? level->getID()
                            : std::to_string(EditorIDs::getID(level));

  std::string key = levelId + "-" + profile.profileName;
  Mod::get()->setSavedValue<std::string>(key, "");
}