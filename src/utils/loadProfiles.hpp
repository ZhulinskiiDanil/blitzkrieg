#pragma once
#include <vector>
#include "../serialization/profile/index.hpp" // Profile, from_json, to_json

// Загружает профили из export.json (в ресурсах мода) и сохраняет в Mod saved data
std::vector<Profile> loadProfiles();