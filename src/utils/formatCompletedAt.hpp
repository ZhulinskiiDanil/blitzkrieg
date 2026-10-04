#pragma once
#include <ctime>
#include <string>

// 2026-09-12 18:40, empty when the timestamp is not set
std::string formatCompletedAt(std::time_t timestamp);

// Sep 12, empty when the timestamp is not set
std::string formatShortDate(std::time_t timestamp);
