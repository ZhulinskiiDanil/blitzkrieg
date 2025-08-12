#pragma once

#include <string>

struct Range {
  std::string id;
  int from;
  int to;
  bool checked;
  std::string note;
  int attempts;
  int completionCounter;
};