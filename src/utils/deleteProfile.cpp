#include "deleteProfile.hpp"
#include "./getProfiles.hpp"

void deleteProfile(const std::string &profileId)
{
  auto profiles = getProfiles();

  // Remove
  auto it = std::remove_if(profiles.begin(), profiles.end(),
                           [&](const Profile &p)
                           { return p.id == profileId; });

  if (it != profiles.end())
  {
    profiles.erase(it, profiles.end());
    geode::log::debug("Deleted profile with id: {}", profileId);

    // Save
    matjson::Value j = profiles;
    Mod::get()->setSavedValue("profiles", j.dump(matjson::NO_INDENTATION));
  }
  else
  {
    geode::log::debug("Profile with id {} not found.", profileId);
  }
}
