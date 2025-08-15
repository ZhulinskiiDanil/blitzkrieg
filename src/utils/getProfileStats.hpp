#pragma once
#include "../serialization/profile/index.hpp"
#include <optional>

struct ProfileStats
{
  std::optional<int> currentStage; // если все пройдены — будет nullopt
  int totalStages;                 // всего этапов
};

// Получаем статистику по профилю
ProfileStats getProfileStats(const Profile &profile);