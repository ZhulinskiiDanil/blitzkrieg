#pragma once

#include <string>
#include <ctime>
#include <string>

struct Range
{
  std::string id;

  float from = 0.f;
  float to = 0.f;

  float firstRunFrom = 0.f;
  float firstRunTo = 0.f;

  float bestRunFrom = 0.f;
  float bestRunTo = 0.f;

  bool checked = false;
  bool consider = true;
  bool automaticallyClosed = false;

  int attempts = 0;
  float timePlayed = 0.f;

  std::string note;

  std::time_t completedAt = 0;

  int attemptsToComplete = 0;
  int completionCounter = 0;
};