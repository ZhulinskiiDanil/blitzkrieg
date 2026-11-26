#pragma once

#include <Geode/Geode.hpp>

#include <ctime>
#include <string>

#include "../../../../serialization/news/BlitzNewsItem.hpp"

using namespace geode::prelude;

class NewsCard : public CCLayer
{
private:
    BlitzNewsItem m_news;
    CCSize m_size;

    void onAction(CCObject *sender);

    static std::string formatRelativeTime(
        std::time_t timestamp);

    static ccColor4B getAccentColor(
        NewsType type);

    static std::string getTypeName(
        NewsType type);

public:
    static NewsCard *create(
        BlitzNewsItem const &news,
        CCSize const &size);

    bool init(
        BlitzNewsItem const &news,
        CCSize const &size);
};