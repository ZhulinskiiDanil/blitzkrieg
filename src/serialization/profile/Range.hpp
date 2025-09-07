#pragma once

#include <string>

struct Range
{
  std::string id;
  float from;
  float to;
  bool checked;
  std::string note;
  int attempts;
  int completionCounter;
};