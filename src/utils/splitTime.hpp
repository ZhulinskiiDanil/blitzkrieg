#pragma once
#include <string>

struct TimeParts
{
  int days;
  int hours;
  int minutes;
  int seconds;
};

TimeParts splitTime(double seconds);
