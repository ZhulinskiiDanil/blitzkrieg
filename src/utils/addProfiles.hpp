#pragma once

#include <vector>
#include <string>
#include <algorithm>

#include <Geode/Geode.hpp>
#include "../serialization/profile/index.hpp"
#include "./getProfiles.hpp"

using namespace geode::prelude;

// Добавляет массив профилей в список profiles (Mod saved value)
// Если профиль с таким же id уже есть — по умолчанию пропускает,
// но если overwrite = true, то перезаписывает.
void addProfiles(const std::vector<Profile> &newProfiles, bool overwrite = false);
