#include "formatCompletedAt.hpp"

#include <Geode/utils/general.hpp>
#include <fmt/chrono.h>

std::string formatCompletedAt(std::time_t timestamp)
{
  if (timestamp <= 0)
    return {};

  return fmt::format("{:%Y-%m-%d %H:%M}", geode::localtime(timestamp));
}

std::string formatShortDate(std::time_t timestamp)
{
  if (timestamp <= 0)
    return {};

  return fmt::format("{:%b %d}", geode::localtime(timestamp));
}
