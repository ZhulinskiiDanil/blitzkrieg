#include "dateKey.hpp"

#include <Geode/utils/general.hpp>
#include <fmt/chrono.h>

std::string getDateKey(std::time_t time)
{
  return fmt::format("{:%Y-%m-%d}", geode::localtime(time));
}

std::time_t getLocalNoon(std::time_t time)
{
  auto local = geode::localtime(time);
  local.tm_hour = 12;
  local.tm_min = 0;
  local.tm_sec = 0;
  local.tm_isdst = -1;

  return std::mktime(&local);
}

int getWeekdayFromMonday(std::time_t time)
{
  // tm_wday is 0 for Sunday
  return (geode::localtime(time).tm_wday + 6) % 7;
}
