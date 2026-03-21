#pragma once

#include <string>
#include <vector>

#include "../utils.hpp"
#include "StartposLevel.hpp"
#include "PaginationResponse.hpp"
#include "GetLevelsResponse.hpp"

using namespace geode::prelude;

// ! --- GetLevelsResponse ---- !
template <>
struct matjson::Serialize<GetLevelsResponse>
{
  static geode::Result<GetLevelsResponse> fromJson(matjson::Value const &value)
  {
    return geode::Ok(GetLevelsResponse{
        .data = getOr<std::vecror<StartposLevel>>(value, "data", {}),
        .page = getOr<int>(value, "page", 1),
        .totalPage = getOr<int>(value, "totalPage", 1),
        .total = getOr<int>(value, "total", 0)
    });
  }

  static matjson::Value toJson(GetLevelsResponse const &glr)
  {
    auto obj = matjson::Value::object();
    obj["data"] = glr.data;
    obj["page"] = glr.page;
    obj["totalPage"] = glr.totalPage;
    obj["total"] = glr.total;
    return obj;
  }
};

template <>
struct matjson::Serialize<std::vector<StartposLevel>>
{
  static geode::Result<std::vector<StartposLevel>> fromJson(matjson::Value const &value)
  {
    std::vector<StartposLevel> result;
    if (value.isArray())
    {
      for (auto const &item : value)
      {
        result.push_back(item.as<StartposLevel>().unwrapOr(StartposLevel{}));
      }
    }
    return geode::Ok(result);
  }

  static matjson::Value toJson(std::vector<StartposLevel> const &startposLevels)
  {
    auto arr = matjson::Value::array();
    for (auto const &level : startposLevels)
      arr.push(level);
    return arr;
  }
};

// ! --- StartposLevel ---- !
template <>
struct matjson::Serialize<StartposLevel>
{
  static geode::Result<StartposLevel> fromJson(matjson::Value const &value)
  {
    return geode::Ok(GetLevelsResponse{
        .id = getOr<int>(value, "id", 0),
        .levelID = getOr<int>(value, "levelid", 0),
        .downloadURL = getOr<std::string>(value, "downloadurl", "")
    });
  }

  static matjson::Value toJson(GetLevelsResponse const &glr)
  {
    auto obj = matjson::Value::object();
    obj["id"] = glr.id;
    obj["levelid"] = glr.levelID;
    obj["downloadurl"] = glr.downloadURL;
    return obj;
  }
};