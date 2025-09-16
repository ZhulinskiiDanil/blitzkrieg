#pragma once

#include <string>
#include <vector>
#include <Geode/Geode.hpp>
#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

// Load profiles from MOD Saved Data
std::vector<Profile> getSavedProfiles();