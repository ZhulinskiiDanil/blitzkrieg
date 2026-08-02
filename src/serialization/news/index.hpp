#pragma once

#include <Geode/Geode.hpp>

#include <vector>

#include "NewsAction.hpp"
#include "NewsItem.hpp"
#include "GetNewsResponse.hpp"

#include "../../utils/getOr.hpp"

using namespace geode::prelude;

// ! --- NewsAction --- !

template <>
struct matjson::Serialize<NewsAction>
{
  static geode::Result<NewsAction> fromJson(
      matjson::Value const &value)
  {
    NewsAction action;

    action.label =
        getOr<std::string>(
            value,
            "label",
            "");

    action.type = newsActionTypeFromString(
        getOr<std::string>(
            value,
            "type",
            "unknown"));

    action.value =
        getOr<std::string>(
            value,
            "value",
            "");

    return geode::Ok(action);
  }

  static matjson::Value toJson(
      NewsAction const &action)
  {
    auto object =
        matjson::Value::object();

    object["label"] =
        action.label;

    object["type"] =
        newsActionTypeToString(
            action.type);

    object["value"] =
        action.value;

    return object;
  }
};

// ! --- NewsItem --- !

template <>
struct matjson::Serialize<NewsItem>
{
  static geode::Result<NewsItem> fromJson(
      matjson::Value const &value)
  {
    NewsItem news;

    news.id =
        getOr<std::string>(
            value,
            "id",
            "");

    news.type = newsTypeFromString(
        getOr<std::string>(
            value,
            "type",
            "unknown"));

    news.title =
        getOr<std::string>(
            value,
            "title",
            "");

    news.description =
        getOr<std::string>(
            value,
            "description",
            "");

    news.publishedAt =
        getOr<std::time_t>(
            value,
            "publishedAt",
            0);

    news.pinned =
        getOr<bool>(
            value,
            "pinned",
            false);

    auto actionsValue =
        value.get("actions");

    if (actionsValue.isOk())
    {
      auto actionsResult =
          actionsValue
              .unwrap()
              .as<std::vector<NewsAction>>();

      if (actionsResult.isErr())
      {
        return geode::Err(
            fmt::format(
                "Failed to parse news actions: {}",
                actionsResult.unwrapErr()));
      }

      news.actions =
          actionsResult.unwrap();
    }

    return geode::Ok(news);
  }

  static matjson::Value toJson(
      NewsItem const &news)
  {
    auto object =
        matjson::Value::object();

    object["id"] =
        news.id;

    object["type"] =
        newsTypeToString(
            news.type);

    object["title"] =
        news.title;

    object["description"] =
        news.description;

    object["publishedAt"] =
        news.publishedAt;

    object["pinned"] =
        news.pinned;

    auto actions =
        matjson::Value::array();

    for (auto const &action :
         news.actions)
    {
      actions.push(action);
    }

    object["actions"] =
        actions;

    return object;
  }
};

// ! --- GetNewsResponse --- !

template <>
struct matjson::Serialize<GetNewsResponse>
{
  static geode::Result<GetNewsResponse> fromJson(
      matjson::Value const &value)
  {
    GetNewsResponse response;

    auto dataValue =
        value.get("data");

    if (dataValue.isErr())
      return geode::Err(
          "Missing news data");

    if (!dataValue.unwrap().isArray())
      return geode::Err(
          "News data must be an array");

    for (auto const &item :
         dataValue.unwrap())
    {
      auto newsResult =
          item.as<NewsItem>();

      if (newsResult.isErr())
      {
        return geode::Err(
            fmt::format(
                "Failed to parse news item: {}",
                newsResult.unwrapErr()));
      }

      response.data.push_back(
          newsResult.unwrap());
    }

    response.page =
        getOr<int>(
            value,
            "page",
            1);

    response.limit =
        getOr<int>(
            value,
            "limit",
            20);

    response.totalPages =
        getOr<int>(
            value,
            "totalPages",
            0);

    response.total =
        getOr<int>(
            value,
            "total",
            0);

    return geode::Ok(response);
  }
};