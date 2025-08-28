#include "saveProfile.hpp"

void saveProfile(const Profile &profile)
{
    // Загружаем текущие сохранённые профили
    auto savedProfilesStr = Mod::get()->getSavedValue<std::string>("profiles", "[]");

    auto res = matjson::parseAs<std::vector<Profile>>(savedProfilesStr);
    std::vector<Profile> profiles;

    if (res.isErr())
    {
        geode::log::debug("Ошибка при парсинге сохранённых профилей: {}", res.unwrapErr());
    }
    else
    {
        profiles = res.unwrap();
    }

    // Ищем профиль с таким же id
    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [&](const Profile &p)
                           { return p.id == profile.id; });

    if (it != profiles.end())
    {
        *it = profile;
        geode::log::debug("Updated profile: {}", profile.profileName);
    }
    else
    {
        profiles.push_back(profile);
        geode::log::debug("Added new profile: {}", profile.profileName);
    }

    // Сохраняем обратно в saved value через matjson
    matjson::Value j = profiles; // автоматически через Serialize<Profile>::toJson
    Mod::get()->setSavedValue("profiles", j.dump(matjson::NO_INDENTATION));
}