#pragma once

#include <vector>
#include "../serialization/profile/index.hpp"

std::vector<Stage> mergeProfileStages(const std::vector<Stage> &s1, const std::vector<Stage> &s2, bool autoCloseRunsIfStagePassed);