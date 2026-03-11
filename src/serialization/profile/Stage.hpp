#pragma once

#include <string>
#include <vector>
#include "Range.hpp"

struct Stage
{
  std::string id;
  int stage;
  bool checked;
  std::string note;
  int completionCounter;
  std::vector<Range> ranges;
};