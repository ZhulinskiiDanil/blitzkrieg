#pragma once

#include <ctime>
#include <map>
#include <string>
#include <vector>
#include "Stage.hpp"

// What happened on one day, local time
struct DayStats
{
  int attempts = 0;
  float timePlayed = 0.f;
  int runsPassed = 0;
  int stagesClosed = 0;
  // Best attempt that started at 0%, percent
  float bestFromZero = 0.f;
};

struct ProfileData
{
  std::vector<float> tags;
  std::vector<Stage> stages;
  // Keyed by local date, YYYY-MM-DD
  std::map<std::string, DayStats> history;
  // When each achievement was unlocked, keyed by its id
  std::map<std::string, std::time_t> achievements;
};
