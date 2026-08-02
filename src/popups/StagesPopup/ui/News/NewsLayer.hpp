#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../../../../serialization/news/index.hpp"

using namespace geode::prelude;

class NewsLayer : public CCLayer
{
private:
    static constexpr std::int64_t CACHE_TTL_SECONDS =
        5 * 60;

    CCSize m_size;
    ScrollLayer *m_scroll = nullptr;

    async::TaskHolder<web::WebResponse> m_newsTask;

    void loadNews(
        bool forceRefresh = false);

    void showMessage(
        std::string const &message);

    std::optional<GetNewsResponse>
    loadNewsCache() const;

    void saveNewsCache(
        web::WebResponse const &response) const;

    bool isNewsCacheFresh() const;

public:
    static NewsLayer *create(
        CCSize const &contentSize);

    bool init(
        CCSize const &contentSize);

    void setNews(
        std::vector<NewsItem> const &news);

    void reload();
};