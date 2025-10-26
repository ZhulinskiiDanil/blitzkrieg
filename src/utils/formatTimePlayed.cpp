#include "formatTimePlayed.hpp"
#include "splitTime.hpp"
#include <fmt/core.h>
#include <string>

std::string formatTimePlayed(double seconds)
{
  TimeParts t = splitTime(seconds);

  bool onlyMinutes = (t.days == 0 && t.hours == 0 && t.minutes > 0);

  std::string result;

  if (t.days > 0)
    result += fmt::format("{} <small>d</small> ", t.days);
  if (t.hours > 0)
    result += fmt::format("{} <small>h</small> ", t.hours);
  if (t.minutes > 0)
    result += fmt::format("{} <small>{}</small> ",
                          t.minutes, onlyMinutes ? "min" : "m");
  if (t.seconds > 0 || result.empty())
    result += fmt::format("{} <small>s</small>", t.seconds);

  if (!result.empty() && result.back() == ' ')
    result.pop_back();

  return result;
}
