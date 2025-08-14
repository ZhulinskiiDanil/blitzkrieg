#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <sstream>

using json = nlohmann::json;

namespace safe
{

  // Безопасное получение числа из JSON, поддерживает числовой и строковый тип
  template <typename T>
  T number(const json &j, const std::string &key)
  {
    if (!j.contains(key))
      return T{};
    if (j[key].is_number())
      return j[key].get<T>();
    if (j[key].is_string())
    {
      try
      {
        std::istringstream iss(j[key].get<std::string>());
        T val{};
        iss >> val;
        return val;
      }
      catch (...)
      {
        return T{};
      }
    }
    return T{};
  }

} // namespace safe