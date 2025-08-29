#include "saveProfiles.hpp"

void saveProfiles(const std::vector<Profile> &profiles)
{
    matjson::Value j = profiles;

    if (j.isArray())
    {
        std::string jsonString = j.dump(matjson::NO_INDENTATION);
        Mod::get()->setSavedValue("profiles", jsonString);
    }

    geode::log::debug("Saved {} profiles", profiles.size());
}