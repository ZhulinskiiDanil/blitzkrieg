#include "saveProfiles.hpp"

void saveProfiles(const std::vector<Profile> &profiles)
{
    matjson::Value j = profiles;
    Mod::get()->setSavedValue("profiles", j.dump(matjson::NO_INDENTATION));

    geode::log::debug("Saved {} profiles", profiles.size());
}