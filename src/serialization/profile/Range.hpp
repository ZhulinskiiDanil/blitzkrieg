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
  std::string note;
  int attempts;
  int completionCounter;
};