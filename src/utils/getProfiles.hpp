#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <Geode/Geode.hpp>
#include "../serialization/profile/index.hpp"

using json = nlohmann::json;
using namespace geode::prelude;

// Загружает профили из Mod saved data
std::vector<Profile> getProfiles();