#include "getNews.hpp"

#include <algorithm>
#include <chrono>

#include "../../config.hpp"

using namespace geode::prelude;

web::WebFuture api::news::getNews(
    int page,
    int limit)
{
  page = std::max(page, 1);
  limit = std::clamp(limit, 1, 100);

  web::WebRequest request;

  request.param(
      "page",
      fmt::format("{}", page));

  request.param(
      "limit",
      fmt::format("{}", limit));

  request.timeout(std::chrono::seconds(15));

  return request.get(
      fmt::format("{}/news", API_URL));
}

geode::Result<GetNewsResponse>
api::news::parseGetNewsResponse(
    web::WebResponse const &response)
{
  if (!response.ok())
  {
    auto responseBody = response.string().unwrapOr(
        std::string("Unknown error"));

    return geode::Err(
        fmt::format(
            "News request failed: HTTP {}: {}",
            response.code(),
            responseBody));
  }

  auto bodyResult = response.string();

  if (bodyResult.isErr())
  {
    return geode::Err(
        fmt::format(
            "Failed to read news response: {}",
            bodyResult.unwrapErr()));
  }

  auto parseResult =
      matjson::parseAs<GetNewsResponse>(
          bodyResult.unwrap());

  if (parseResult.isErr())
  {
    return geode::Err(
        fmt::format(
            "Failed to parse news response: {}",
            parseResult.unwrapErr()));
  }

  return geode::Ok(parseResult.unwrap());
}