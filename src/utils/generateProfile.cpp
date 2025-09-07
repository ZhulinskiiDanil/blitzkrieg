#include "generateProfile.hpp"

matjson::Value generateProfile(const std::string &profileName, const std::vector<float> &sps)
{
  Profile profile;

  profile.id = generateUUID();
  profile.profileName = profileName;

  // Записываем sps в tags (float -> int)
  profile.data.tags.clear();
  for (float v : sps)
  {
    profile.data.tags.push_back(static_cast<int>(v));
  }

  // Генерируем stages через generateStages
  matjson::Value stagesJson = generateStages(sps);
  profile.data.stages = stagesJson.as<std::vector<Stage>>().unwrap();

  // Преобразуем в matjson::Value
  return matjson::Serialize<Profile>::toJson(profile);
}
