#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// Additional function for safely parsing value
template <class T>
T getOr(matjson::Value const &value, std::string_view key, T fallback)
{
  if (auto val = value.get(key))
  {
    auto v = val.unwrap().as<T>();
    return v.unwrapOr(fallback);
  }

  return fallback;
}