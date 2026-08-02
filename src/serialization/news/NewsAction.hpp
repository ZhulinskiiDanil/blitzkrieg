#pragma once

#include <string>

enum class NewsActionType
{
  Unknown,
  OpenLevel,
  OpenURL,
  CopyText
};

struct NewsAction
{
  std::string label;
  NewsActionType type = NewsActionType::Unknown;
  std::string value;
};

inline NewsActionType newsActionTypeFromString(
    std::string const &value)
{
  if (value == "open-level")
    return NewsActionType::OpenLevel;

  if (value == "open-url")
    return NewsActionType::OpenURL;

  if (value == "copy-text")
    return NewsActionType::CopyText;

  return NewsActionType::Unknown;
}

inline std::string newsActionTypeToString(
    NewsActionType type)
{
  switch (type)
  {
  case NewsActionType::OpenLevel:
    return "open-level";

  case NewsActionType::OpenURL:
    return "open-url";

  case NewsActionType::CopyText:
    return "copy-text";

  default:
    return "unknown";
  }
}