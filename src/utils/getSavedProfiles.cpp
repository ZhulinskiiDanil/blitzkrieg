#include "getSavedProfiles.hpp"
#include "../serialization/profile/index.hpp"

std::vector<Profile> getSavedProfiles()
{
    std::string saved = Mod::get()->getSavedValue<std::string>("profiles", "[]");

    auto res = matjson::parseAs<std::vector<Profile>>(saved);
    if (res.isErr())
    {
        geode::log::error("Profiles JSON parse error: {}", res.unwrapErr());
        return {};
    }

    return res.unwrap();
}