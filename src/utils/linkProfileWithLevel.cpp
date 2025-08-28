#include "linkProfileWithLevel.hpp"
#include "./unlinkProfileFromLevel.hpp"
#include "./getProfiles.hpp"

void linkProfileWithLevel(const Profile &profile, GJGameLevel *level)
{
  if (!level)
    return geode::log::debug("Failed to find level in \"linkProfileWithLevel\"");

  if (!&profile)
    return geode::log::debug("Failed to find profile in \"linkProfileWithLevel\"");

  // Уникальный идентификатор уровня
  std::string lvlId = !level->getID().empty()
                          ? level->getID()
                          : std::to_string(EditorIDs::getID(level));

  geode::log::debug("Link profile {} with level {}", profile.id, lvlId);

  // Отвязываем этот уровень от всех других профилей
  auto profiles = getProfiles();

  // Unlink all profile(s) from current level
  for (const auto &p : profiles)
  {
    unlinkProfileFromLevel(p, lvlId);
  }

  // Link new profile
  std::string key = lvlId + "-" + profile.profileName;
  matjson::Value profileJson = profile;
  Mod::get()->setSavedValue(key, profileJson.dump(matjson::NO_INDENTATION));
}
