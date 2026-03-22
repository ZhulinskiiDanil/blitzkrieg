#pragma once

#include "../utils.hpp"
#include "StartPosLevel.hpp"
#include "PaginationResponse.hpp"
#include "GetLevelsResponse.hpp"

using namespace geode::prelude;

// ! --- GetLevelsResponse ---- !
template <typename T>
struct matjson::Serialize<PaginationResponse<T>>
{
  static geode::Result<PaginationResponse> fromJson(matjson::Value const &value)
  {
    return geode::Ok(PaginationResponse{
        .data = getOr<T>(value, "data", {}),
        .page = getOr<int>(value, "page", 1),
        .totalPage = getOr<int>(value, "totalPage", 1),
        .total = getOr<int>(value, "total", 0)});
  }

  static matjson::Value toJson(PaginationResponse const &glr)
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
struct matjson::Serialize<std::vector<StartPosLevel>>
{
  static geode::Result<std::vector<StartPosLevel>> fromJson(matjson::Value const &value)
  {
    std::vector<StartPosLevel> result;
    if (value.isArray())
    {
      for (auto const &item : value)
      {
        result.push_back(item.as<StartPosLevel>().unwrapOr(StartPosLevel{}));
      }
    }
    return geode::Ok(result);
  }

  static matjson::Value toJson(std::vector<StartPosLevel> const &StartPosLevels)
  {
    auto arr = matjson::Value::array();
    for (auto const &level : StartPosLevels)
      arr.push(level);
    return arr;
  }
};

// ! --- StartPosLevel ---- !
template <>
struct matjson::Serialize<StartPosLevel>
{
  static geode::Result<StartPosLevel> fromJson(matjson::Value const &value)
  {
    return geode::Ok(GetLevelsResponse{
        .id = getOr<int>(value, "id", 0),
        .levelID = getOr<int>(value, "levelid", 0),
        .downloadURL = getOr<std::string>(value, "downloadurl", "")});
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