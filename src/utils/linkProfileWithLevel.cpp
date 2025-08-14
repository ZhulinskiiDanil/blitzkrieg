#include "linkProfileWithLevel.hpp"
#include "./unlinkProfileFromLevel.hpp"
#include "./getProfiles.hpp"

void linkProfileWithLevel(const Profile &profile, GJGameLevel *level)
{
  if (!level)
    return;

  // Уникальный идентификатор уровня
  std::string levelId = !level->getID().empty()
                            ? level->getID()
                            : std::to_string(EditorIDs::getID(level));

  // Отвязываем этот уровень от всех других профилей
  auto allProfiles = getProfiles();
  for (const auto &p : allProfiles)
  {
    if (p.profileName != profile.profileName)
    {
      // Проверяем, привязан ли профиль к этому уровню
      // Предполагаем, что функция `isProfileLinkedToLevel` есть или делаем через ключ
      std::string key = levelId + "-" + p.profileName;
      auto saved = Mod::get()->getSavedValue<std::string>(key);

      if (!saved.empty())
      {
        unlinkProfileFromLevel(p, levelId);
      }
    }
  }

  // Привязываем текущий профиль
  std::string key = levelId + "-" + profile.profileName;
  json j = serializeProfile(profile);
  Mod::get()->setSavedValue(key, j.dump());
}
