#include "./getProfileByLevel.hpp"

Profile getProfileByLevel(GJGameLevel *level)
{
  if (!level)
    return Profile{};
  // Получаем уникальный ID уровня
  std::string levelId = !level->getID().empty()
                            ? level->getID()
                            : geode::utils::numToString(EditorIDs::getID(level));
  return getProfileByLevel(levelId);
}

Profile getProfileByLevel(const std::string &levelId)
{
  // Получаем все профили
  auto profiles = getProfiles();
  for (const auto &profile : profiles)
  {
    std::string key = levelId + "-" + profile.profileName;
    auto savedStr = Mod::get()->getSavedValue<std::string>(key);

    if (!savedStr.empty())
    {
      return profile;
    }
  }
  return Profile{}; // пустой профиль, если ничего не нашли
}