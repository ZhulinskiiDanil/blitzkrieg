#pragma once

#include <sstream>
#include <iomanip>
#include <string>

inline std::string encodeURIComponent(const std::string &str)
{
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (unsigned char c : str)
  {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      escaped << c;
    }
    else
    {
      escaped << '%' << std::uppercase
              << std::setw(2)
              << int(c);
    }
  }

  return escaped.str();
}