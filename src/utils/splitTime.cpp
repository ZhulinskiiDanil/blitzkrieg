#include "splitTime.hpp"

TimeParts splitTime(double seconds)
{
  int total = static_cast<int>(seconds);

  TimeParts t;
  t.days = total / 86400;
  t.hours = (total % 86400) / 3600;
  t.minutes = (total % 3600) / 60;
  t.seconds = total % 60;

  return t;
}
