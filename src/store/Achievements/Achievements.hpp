#pragma once

#include <functional>
#include <string>
#include <vector>

#include "../../serialization/profile/index.hpp"

// One achievement of a profile, unlocked when value(profile) reaches target
struct AchievementDef
{
  const char *id;
  const char *title;
  const char *description;
  // GD sprite frame
  const char *icon;
  float target;
  std::function<float(Profile const &)> value;
};

// Every achievement, in the order they are usually unlocked
std::vector<AchievementDef> const &getAchievements();

// Unlocks what the profile reached, returns the new ones
std::vector<AchievementDef const *> updateAchievements(Profile &profile);

// ! --- Daily goal, one for the whole mod --- !

enum class DailyGoalType
{
  Runs,
  Attempts,
  Minutes
};

DailyGoalType getDailyGoalType();
void setDailyGoalType(DailyGoalType type);

int getDailyGoalAmount();
void setDailyGoalAmount(int amount);

// Steps of the - and + buttons and the limits of the amount
int getDailyGoalStep(DailyGoalType type);
int getDailyGoalMin(DailyGoalType type);
int getDailyGoalMax(DailyGoalType type);

// Today, summed over every profile
float getDailyGoalProgress();

// True once a day, when the goal is reached
bool checkDailyGoalReached();
