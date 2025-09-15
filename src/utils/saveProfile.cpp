#include "saveProfile.hpp"

void saveProfile(const Profile &profile)
{
    auto savedProfilesStr = Mod::get()->getSavedValue<std::string>("profiles", "[]");

    auto res = matjson::parseAs<std::vector<Profile>>(savedProfilesStr);
    std::vector<Profile> profiles;

    if (res.isErr())
        geode::log::error("Parsing error of saved profiles: {}", res.unwrapErr());
    else
        profiles = res.unwrap();

    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [&](const Profile &p)
                           { return p.id == profile.id; });

    if (it != profiles.end())
        *it = profile;
    else
        profiles.push_back(profile);

    matjson::Value j = profiles;
    Mod::get()->setSavedValue("profiles", j.dump(matjson::NO_INDENTATION));
}