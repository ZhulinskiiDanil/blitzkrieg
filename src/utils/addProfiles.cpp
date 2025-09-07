#include "addProfiles.hpp"

void addProfiles(const std::vector<Profile> &newProfiles, bool overwrite)
{
  auto profiles = getProfiles();

  for (const auto &profile : newProfiles)
  {
    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [&](const Profile &p)
                           { return p.id == profile.id; });

    if (it != profiles.end() && overwrite)
      *it = profile;
    else
      profiles.push_back(profile);
  }

  matjson::Value j = profiles;
  Mod::get()->setSavedValue("profiles", j.dump(matjson::NO_INDENTATION));
}
