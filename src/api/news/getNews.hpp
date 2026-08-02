#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

#include "../../serialization/news/index.hpp"

namespace api::news
{
  geode::utils::web::WebFuture getNews(
      int page = 1,
      int limit = 20);

  geode::Result<GetNewsResponse> parseGetNewsResponse(
      geode::utils::web::WebResponse const &response);
}