#pragma once

#include <string>
#include <vector>
#include "Range.hpp"

struct Stage
{
  std::string id;

  int stage = 0;
  bool checked = false;

  std::string note;

  int completionCounter = 0;

  std::vector<Range> ranges;
};