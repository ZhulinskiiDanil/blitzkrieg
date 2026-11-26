#pragma once

#include <vector>

#include "BlitzNewsItem.hpp"

struct GetNewsResponse
{
  std::vector<BlitzNewsItem> data;

  int page = 1;
  int limit = 20;
  int totalPages = 0;
  int total = 0;
};