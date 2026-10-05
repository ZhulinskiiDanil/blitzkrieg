#include "Achievements.hpp"

#include <algorithm>
#include <ctime>

#include "../GlobalStore.hpp"
#include "../../utils/dateKey.hpp"
#include "../../utils/getMetaInfoFromStages.hpp"
#include "../../utils/isStageDeepChecked.hpp"

using namespace geode::prelude;

namespace
{
  constexpr std::time_t DAY = 24 * 60 * 60;

  const char *GOAL_TYPE_KEY = "daily-goal-type";
  const char *GOAL_AMOUNT_KEY = "daily-goal-amount";
  const char *GOAL_REACHED_KEY = "daily-goal-reached-date";

  // ! --- Values of a profile --- !

  float getTotalAttempts(Profile const &profile)
  {
    float total = 0.f;

    for (auto const &stage : profile.data.stages)
    {
      for (auto const &range : stage.ranges)
        total += range.attempts;
    }

    return total;
  }

  float getTotalHours(Profile const &profile)
  {
    float seconds = 0.f;

    for (auto const &stage : profile.data.stages)
    {
      for (auto const &range : stage.ranges)
        seconds += range.timePlayed;
    }

    return seconds / 3600.f;
  }

  // Runs closed by playing, not by a regenerate
  float getPassedRuns(Profile const &profile)
  {
    float passed = 0.f;

    for (auto const &stage : profile.data.stages)
    {
      for (auto const &range : stage.ranges)
      {
        if (range.consider && range.checked && !range.automaticallyClosed)
          passed++;
      }
    }

    return passed;
  }

  // Stages are counted like the Stage Browser does
  std::pair<int, int> getStages(Profile const &profile)
  {
    int total = 0;
    int completed = 0;

    for (auto const &stage : profile.data.stages)
    {
      if (!isStageConsidered(stage))
        continue;

      total++;

      if (isStageDeepChecked(stage))
        completed++;
    }

    return {completed, total};
  }

  float getStageShare(Profile const &profile)
  {
    const auto [completed, total] = getStages(profile);
    return total > 0 ? 100.f * completed / total : 0.f;
  }

  float getBestFromZero(Profile const &profile)
  {
    float best = 0.f;

    for (auto const &[key, day] : profile.data.history)
      best = std::max(best, day.bestFromZero);

    return best;
  }

  float getBusiestDay(Profile const &profile)
  {
    int busiest = 0;

    for (auto const &[key, day] : profile.data.history)
      busiest = std::max(busiest, day.attempts);

    return static_cast<float>(busiest);
  }

  // Noon of a YYYY-MM-DD key, 0 when it does not parse
  std::time_t parseDateKey(std::string const &key)
  {
    if (key.size() != 10 || key[4] != '-' || key[7] != '-')
      return 0;

    std::tm tm{};
    tm.tm_year = std::atoi(key.substr(0, 4).c_str()) - 1900;
    tm.tm_mon = std::atoi(key.substr(5, 2).c_str()) - 1;
    tm.tm_mday = std::atoi(key.substr(8, 2).c_str());
    tm.tm_hour = 12;
    tm.tm_isdst = -1;

    return std::mktime(&tm);
  }

  // Most days in a row with at least one attempt
  float getLongestStreak(Profile const &profile)
  {
    int longest = 0;
    int current = 0;
    std::time_t previous = 0;

    // Keys are sorted, so the days go in order
    for (auto const &[key, day] : profile.data.history)
    {
      if (day.attempts <= 0)
        continue;

      const auto noon = parseDateKey(key);

      // Noon to noon is a day, give or take an hour of daylight saving
      const bool next = previous > 0 && std::llabs(noon - previous - DAY) <= 2 * 60 * 60;

      current = next ? current + 1 : 1;
      longest = std::max(longest, current);
      previous = noon;
    }

    return static_cast<float>(longest);
  }

  float hasFirstTryRun(Profile const &profile)
  {
    for (auto const &stage : profile.data.stages)
    {
      for (auto const &range : stage.ranges)
      {
        if (range.checked && !range.automaticallyClosed && range.attemptsToComplete == 1)
          return 1.f;
      }
    }

    return 0.f;
  }

  float getMostRunAttempts(Profile const &profile)
  {
    int most = 0;

    for (auto const &stage : profile.data.stages)
    {
      for (auto const &range : stage.ranges)
        most = std::max(most, range.attempts);
    }

    return static_cast<float>(most);
  }

  int getDefaultAmount(DailyGoalType type)
  {
    switch (type)
    {
    case DailyGoalType::Runs:
      return 3;
    case DailyGoalType::Minutes:
      return 60;
    default:
      return 200;
    }
  }
}

// ! --- Achievements --- !

std::vector<AchievementDef> const &getAchievements()
{
  static const std::vector<AchievementDef> achievements = {
      // Runs
      {"first-run", "First Run", "Pass your first run", "GJ_completesIcon_001.png", 1.f, getPassedRuns},
      {"first-try", "First Try", "Pass a run on its first attempt", "GJ_completesIcon_001.png", 1.f, hasFirstTryRun},
      {"runs-10", "Getting There", "Pass 10 runs", "GJ_completesIcon_001.png", 10.f, getPassedRuns},
      {"runs-50", "Run Collector", "Pass 50 runs", "GJ_completesIcon_001.png", 50.f, getPassedRuns},
      {"runs-100", "Run Machine", "Pass 100 runs", "GJ_completesIcon_001.png", 100.f, getPassedRuns},

      // Stages
      {"first-stage", "First Stage", "Complete your first stage", "GJ_starsIcon_001.png", 1.f,
       [](Profile const &p)
       { return static_cast<float>(getStages(p).first); }},
      {"stages-25", "A Quarter Done", "Complete 25% of the stages", "GJ_starsIcon_001.png", 25.f, getStageShare},
      {"stages-50", "Halfway", "Complete 50% of the stages", "GJ_starsIcon_001.png", 50.f, getStageShare},
      {"stages-75", "Almost There", "Complete 75% of the stages", "GJ_starsIcon_001.png", 75.f, getStageShare},
      {"stages-100", "Ready to Verify", "Complete every stage", "GJ_starsIcon_001.png", 100.f, getStageShare},

      // Best from 0%
      {"best-25", "Quarter From Zero", "Reach 25% from 0%", "GJ_starsIcon_001.png", 25.f, getBestFromZero},
      {"best-50", "Half From Zero", "Reach 50% from 0%", "GJ_starsIcon_001.png", 50.f, getBestFromZero},
      {"best-75", "So Close", "Reach 75% from 0%", "GJ_starsIcon_001.png", 75.f, getBestFromZero},

      // Attempts
      {"attempts-1000", "Warmed Up", "Play 1,000 attempts", "miniSkull_001.png", 1000.f, getTotalAttempts},
      {"attempts-5000", "Dedicated", "Play 5,000 attempts", "miniSkull_001.png", 5000.f, getTotalAttempts},
      {"attempts-10000", "Relentless", "Play 10,000 attempts", "miniSkull_001.png", 10000.f, getTotalAttempts},
      {"grinder", "Grinder", "Play 500 attempts on a single run", "miniSkull_001.png", 500.f, getMostRunAttempts},
      {"day-500", "Marathon", "Play 500 attempts in one day", "miniSkull_001.png", 500.f, getBusiestDay},

      // Time
      {"hours-1", "First Hour", "Play for an hour", "GJ_timeIcon_001.png", 1.f, getTotalHours},
      {"hours-10", "Ten Hours In", "Play for 10 hours", "GJ_timeIcon_001.png", 10.f, getTotalHours},
      {"hours-50", "Committed", "Play for 50 hours", "GJ_timeIcon_001.png", 50.f, getTotalHours},

      // Streaks
      {"streak-3", "Habit", "Play 3 days in a row", "GJ_timeIcon_001.png", 3.f, getLongestStreak},
      {"streak-7", "Full Week", "Play 7 days in a row", "GJ_timeIcon_001.png", 7.f, getLongestStreak},
      {"streak-30", "Unstoppable", "Play 30 days in a row", "GJ_timeIcon_001.png", 30.f, getLongestStreak},
  };

  return achievements;
}

std::vector<AchievementDef const *> updateAchievements(Profile &profile)
{
  std::vector<AchievementDef const *> unlocked;
  const auto now = std::time(nullptr);

  for (auto const &achievement : getAchievements())
  {
    if (profile.data.achievements.contains(achievement.id))
      continue;

    if (achievement.value(profile) + .0001f < achievement.target)
      continue;

    profile.data.achievements[achievement.id] = now;
    unlocked.push_back(&achievement);
  }

  return unlocked;
}

// ! --- Daily goal --- !

DailyGoalType getDailyGoalType()
{
  const auto type = Mod::get()->getSavedValue<std::string>(GOAL_TYPE_KEY, "attempts");

  if (type == "runs")
    return DailyGoalType::Runs;
  if (type == "minutes")
    return DailyGoalType::Minutes;

  return DailyGoalType::Attempts;
}

void setDailyGoalType(DailyGoalType type)
{
  const char *saved = "attempts";

  if (type == DailyGoalType::Runs)
    saved = "runs";
  else if (type == DailyGoalType::Minutes)
    saved = "minutes";

  Mod::get()->setSavedValue<std::string>(GOAL_TYPE_KEY, saved);

  // Amounts of different types have nothing in common
  setDailyGoalAmount(getDefaultAmount(type));
}

int getDailyGoalAmount()
{
  const auto type = getDailyGoalType();
  const auto amount = Mod::get()->getSavedValue<int64_t>(GOAL_AMOUNT_KEY, getDefaultAmount(type));

  return std::clamp(static_cast<int>(amount), getDailyGoalMin(type), getDailyGoalMax(type));
}

void setDailyGoalAmount(int amount)
{
  const auto type = getDailyGoalType();
  Mod::get()->setSavedValue<int64_t>(GOAL_AMOUNT_KEY, std::clamp(amount, getDailyGoalMin(type), getDailyGoalMax(type)));
}

int getDailyGoalStep(DailyGoalType type)
{
  switch (type)
  {
  case DailyGoalType::Runs:
    return 1;
  case DailyGoalType::Minutes:
    return 15;
  default:
    return 25;
  }
}

int getDailyGoalMin(DailyGoalType type)
{
  return getDailyGoalStep(type);
}

int getDailyGoalMax(DailyGoalType type)
{
  switch (type)
  {
  case DailyGoalType::Runs:
    return 50;
  case DailyGoalType::Minutes:
    return 600;
  default:
    return 5000;
  }
}

float getDailyGoalProgress()
{
  const auto key = getDateKey(std::time(nullptr));
  const auto type = getDailyGoalType();
  float progress = 0.f;

  for (auto const &profile : GlobalStore::get()->getProfiles())
  {
    auto it = profile.data.history.find(key);

    if (it == profile.data.history.end())
      continue;

    switch (type)
    {
    case DailyGoalType::Runs:
      progress += it->second.runsPassed;
      break;
    case DailyGoalType::Minutes:
      progress += it->second.timePlayed / 60.f;
      break;
    default:
      progress += it->second.attempts;
      break;
    }
  }

  return progress;
}

bool checkDailyGoalReached()
{
  const auto today = getDateKey(std::time(nullptr));

  if (Mod::get()->getSavedValue<std::string>(GOAL_REACHED_KEY, "") == today)
    return false;

  if (getDailyGoalProgress() < getDailyGoalAmount())
    return false;

  Mod::get()->setSavedValue<std::string>(GOAL_REACHED_KEY, today);
  return true;
}
