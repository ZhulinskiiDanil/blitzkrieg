#pragma once

#include <vector>
#include <string>
#include <algorithm>

#include <Geode/Geode.hpp>
#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

// Сохраняет переданный профиль в список profiles (Mod saved value)
// Если профиль с таким же id уже есть — перезаписывает его.
void saveProfile(const Profile &profile);