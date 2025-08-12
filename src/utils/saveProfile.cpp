#include "saveProfile.hpp"

#include <Geode/Geode.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>

using namespace geode::prelude;
using json = nlohmann::json;

void saveProfile(const Profile& profile) {
    try {
        // Загружаем текущие сохранённые профили
        auto savedProfilesStr = Mod::get()->getSavedValue<std::string>("profiles");
        std::vector<Profile> profiles;

        if (!savedProfilesStr.empty()) {
            profiles = json::parse(savedProfilesStr).get<std::vector<Profile>>();
        }

        // Ищем профиль с таким же id
        auto it = std::find_if(profiles.begin(), profiles.end(),
            [&](const Profile& p) { return p.id == profile.id; });

        if (it != profiles.end()) {
            *it = profile;
            geode::log::debug("Updated profile: {}", profile.profileName);
        } else {
            profiles.push_back(profile);
            geode::log::debug("Added new profile: {}", profile.profileName);
        }

        // Сериализуем через наши функции
        json j = json::array();
        for (const auto& p : profiles) {
            j.push_back(serializeProfile(p));
        }

        // Сохраняем обратно в saved value
        Mod::get()->setSavedValue("profiles", j.dump());

    } catch (const std::exception& e) {
        geode::log::error("Error saving profile {}: {}", profile.profileName, e.what());
    }
}