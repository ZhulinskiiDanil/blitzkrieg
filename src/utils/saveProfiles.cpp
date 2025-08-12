#include "saveProfiles.hpp"

#include <Geode/Geode.hpp>
#include <nlohmann/json.hpp>

using namespace geode::prelude;
using json = nlohmann::json;

void saveProfiles(const std::vector<Profile>& profiles) {
    try {
        json j = json::array();
        for (const auto& p : profiles) {
            j.push_back(serializeProfile(p));
        }

        // Сохраняем массив профилей в saved value
        Mod::get()->setSavedValue("profiles", j.dump());

        geode::log::debug("Saved {} profiles", profiles.size());
    }
    catch (const std::exception& e) {
        geode::log::error("Error saving profiles: {}", e.what());
    }
}
