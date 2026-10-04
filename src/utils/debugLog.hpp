#pragma once

#include <Geode/Geode.hpp>

#include <utility>

// Returns true when "debug-mode" is enabled in the mod settings.
bool isDebugMode();

// Diagnostic logs that are written only while debug mode is enabled.
// Arguments are not formatted at all when debug mode is off.
namespace debugLog
{
  template <typename... Args>
  inline void info(geode::format::FmtStr<Args...> str, Args &&...args)
  {
    if (isDebugMode())
      geode::log::info(str, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline void warn(geode::format::FmtStr<Args...> str, Args &&...args)
  {
    if (isDebugMode())
      geode::log::warn(str, std::forward<Args>(args)...);
  }
}
