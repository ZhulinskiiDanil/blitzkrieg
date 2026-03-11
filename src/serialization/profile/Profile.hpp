#pragma once

#include <string>
#include "ProfileData.hpp"

struct Profile
{
  std::string id;
  std::string profileName;
  std::string discordWebhookForRunNotifications;
  bool discordWebhookForRunNotificationsEnabled;
  ProfileData data;
};