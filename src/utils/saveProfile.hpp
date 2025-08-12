#pragma once

#include <vector>
#include <string>
#include "../serialization/profile/index.hpp"

// Сохраняет переданный профиль в список profiles (Mod saved value)
// Если профиль с таким же id уже есть — перезаписывает его.
void saveProfile(const Profile& profile);