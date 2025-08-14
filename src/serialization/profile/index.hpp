#pragma once

#include <string>
#include <sstream>

#include <nlohmann/json.hpp>
#include "Range.hpp"
#include "Stage.hpp"
#include "ProfileData.hpp"
#include "Profile.hpp"

#include "../safe/index.hpp"

using json = nlohmann::json;

// to_json
json serializeRange(const Range &range);
json serializeStage(const Stage &stage);
json serializeProfileData(const ProfileData &data);
json serializeProfile(const Profile &profile);

// from_json
void from_json(const json &j, Range &r);
void from_json(const json &j, Stage &s);
void from_json(const json &j, ProfileData &pd);
void from_json(const json &j, Profile &p);