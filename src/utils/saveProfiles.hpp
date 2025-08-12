#pragma once

#include <vector>
#include <string>
#include "../serialization/profile/index.hpp"

// Полностью заменяет сохранённый список профилей на переданный.
void saveProfiles(const std::vector<Profile> &profiles);