#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "./NewsAction.hpp"

enum class NewsType
{
  Unknown,
  Announcement,
  StartPosPublished,
  ModUpdate,
  Warning
};

struct NewsItem
{
  std::string id;
  NewsType type = NewsType::Unknown;

  std::string title;
  std::string description;

  std::time_t publishedAt = 0;
  bool pinned = false;

  std::vector<NewsAction> actions;
};

inline NewsType newsTypeFromString(
    std::string const &value)
{
  if (value == "announcement")
    return NewsType::Announcement;

  if (value == "startpos-published")
    return NewsType::StartPosPublished;

  if (value == "mod-update")
    return NewsType::ModUpdate;

  if (value == "warning")
    return NewsType::Warning;

  return NewsType::Unknown;
}

inline std::string newsTypeToString(
    NewsType type)
{
  switch (type)
  {
  case NewsType::Announcement:
    return "announcement";

  case NewsType::StartPosPublished:
    return "startpos-published";

  case NewsType::ModUpdate:
    return "mod-update";

  case NewsType::Warning:
    return "warning";

  default:
    return "unknown";
  }
}