#pragma once

#include "../utils.hpp"
#include "StartPosLevel.hpp"
#include "PaginationResponse.hpp"
#include "GetLevelsResponse.hpp"

using namespace geode::prelude;

// ! --- StartPosLevel ---- !
template <>
struct matjson::Serialize<StartPosLevel>
{
  static geode::Result<StartPosLevel> fromJson(matjson::Value const &value)
  {
    return geode::Ok(StartPosLevel{
        .id = getOr<int>(value, "id", -1),
        .levelId = getOr<int>(value, "level_id", -1),
        .originalId = getOr<int>(value, "original_id", -1),
        .downloadUrl = getOr<std::string>(value, "download_url", ""),
        .updatedAt = getOr<std::string>(value, "updated_at", ""),
        .createdAt = getOr<std::string>(value, "created_at", "")});
  }

  static matjson::Value toJson(StartPosLevel const &glr)
  {
    auto obj = matjson::Value::object();
    obj["id"] = glr.id;
    obj["level_id"] = glr.levelId;
    obj["original_id"] = glr.originalId;
    obj["created_id"] = glr.createdAt;
    obj["updated_id"] = glr.updatedAt;
    obj["download_url"] = glr.downloadUrl;
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

  static matjson::Value toJson(std::vector<StartPosLevel> const &levels)
  {
    auto arr = matjson::Value::array();
    for (auto const &l : levels)
      arr.push(l);
    return arr;
  }
};

// ! --- PaginationResponse --- !
template <typename T>
struct matjson::Serialize<PaginationResponse<T>>
{
  static geode::Result<PaginationResponse<T>> fromJson(matjson::Value const &value)
  {
    return geode::Ok(PaginationResponse{
        .data = getOr<T>(value, "data", {}),
        .page = getOr<int>(value, "page", 0),
        .totalPages = getOr<int>(value, "totalPages", 0),
        .total = getOr<int>(value, "total", 0)});
  }

  static matjson::Value toJson(PaginationResponse<T> const &glr)
  {
    auto obj = matjson::Value::object();
    obj["data"] = glr.data;
    obj["page"] = glr.page;
    obj["totalPages"] = glr.totalPages;
    obj["total"] = glr.total;
    return obj;
  }
};