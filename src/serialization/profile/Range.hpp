#pragma once

#include <string>

struct Range
{
  std::string id;
  float from;
  float to;
  float firstRunFrom;
  float firstRunTo;
  float bestRunFrom;
  float bestRunTo;
  bool checked;
  int attempts;
  std::string note;
  std::time_t completedAt;
  int attemptsToComplete;
  int completionCounter;
};