#pragma once

#include <vector>

#include "NewsItem.hpp"

struct GetNewsResponse
{
  std::vector<NewsItem> data;

  int page = 1;
  int limit = 20;
  int totalPages = 0;
  int total = 0;
};