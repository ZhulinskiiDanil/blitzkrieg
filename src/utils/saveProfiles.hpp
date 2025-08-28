#pragma once

#include <vector>
#include <string>

#include <Geode/Geode.hpp>
#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

// Completely replaces the saved list of profiles to the transmitted.
void saveProfiles(const std::vector<Profile> &profiles);