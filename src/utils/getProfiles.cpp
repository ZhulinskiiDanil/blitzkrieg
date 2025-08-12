#include "getProfiles.hpp"
#include "../serialization/profile/index.hpp"

std::vector<Profile> getProfiles() {
    auto jsonStr = Mod::get()->getSavedValue<std::string>("profiles");

    if (jsonStr.empty()) {
        return {}; // если ничего не сохранено
    }

    auto j = json::parse(jsonStr);
    return j.get<std::vector<Profile>>();
}