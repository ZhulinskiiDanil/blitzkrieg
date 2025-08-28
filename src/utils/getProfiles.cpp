#include "getProfiles.hpp"
#include "../serialization/profile/index.hpp"

std::vector<Profile> getProfiles()
{
    std::string saved = Mod::get()->getSavedValue<std::string>("profiles", "[]");

    // Используем parseAs для прямого парсинга в вектор Profile
    auto res = matjson::parseAs<std::vector<Profile>>(saved);
    if (res.isErr())
    {
        geode::log::debug("Profiles JSON parse error: {}", res.unwrapErr());
        return {};
    }

    return res.unwrap();
}