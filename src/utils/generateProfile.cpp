#include "generateProfile.hpp"

matjson::Value generateProfile(const std::string &profileName, const std::vector<float> &sps)
{
  Profile profile;

  profile.id = generateUUID();
  profile.profileName = profileName;
  profile.discordWebhookForRunNotifications = "";
  profile.discordWebhookForRunNotificationsEnabled = false;

  profile.data.tags.clear();
  for (float v : sps)
  {
    profile.data.tags.push_back(static_cast<int>(v));
  }

  matjson::Value stagesJson = generateStages(sps);
  profile.data.stages = stagesJson.as<std::vector<Stage>>().unwrap();

  return matjson::Serialize<Profile>::toJson(profile);
}
