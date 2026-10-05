#pragma once
#include <ctime>
#include <string>

// Local date, YYYY-MM-DD, the key of ProfileData::history
std::string getDateKey(std::time_t time);

// Noon of the same local day. Whole days added to it stay
// on the right date when the clock moves for daylight saving.
std::time_t getLocalNoon(std::time_t time);

// Local day of the week, 0 is Monday
int getWeekdayFromMonday(std::time_t time);
