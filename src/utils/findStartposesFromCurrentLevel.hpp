#pragma once

#include <vector>
#include <algorithm>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct StartPosesResult
{
  std::vector<float> percentages_2_1;
  std::vector<float> percentages_2_2;
};

StartPosesResult findStartposesFromCurrentLevel();