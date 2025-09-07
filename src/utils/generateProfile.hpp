#pragma once

#include <vector>
#include <string>
#include <Geode/Geode.hpp>
#include "../serialization/profile/index.hpp"
#include "generateStages.hpp"

matjson::Value generateProfile(const std::string &profileName, const std::vector<float> &sps);
