#include "NewsLayer.hpp"
#include "NewsCard.hpp"

#include "../../../../api/news/getNews.hpp"
#include "../../../../ui/types/index.hpp"
#include "../../../../ui/RectNode.hpp"

#include <ctime>

NewsLayer *NewsLayer::create(
    CCSize const &contentSize)
{
    auto ret = new NewsLayer();

    if (ret && ret->init(contentSize))
    {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool NewsLayer::init(
    CCSize const &contentSize)
{
    if (!CCLayer::init())
        return false;

    UIPadding padding{
        15.f,
        15.f,
        15.f,
        15.f,
    };

    m_size = contentSize;

    this->setContentSize(m_size);

    auto innerSize = CCSize{
        m_size.width -
            padding.left -
            padding.right,

        m_size.height -
            padding.top -
            padding.bottom,
    };

    // ! --- Background --- !

    auto background = RectNode::create(
        innerSize,
        ccc4FFromccc4B({
            30,
            30,
            30,
            255,
        }),
        8.f);

    background->ignoreAnchorPointForPosition(false);
    background->setAnchorPoint({0.5f, 0.5f});

    background->setPosition({
        padding.left +
            innerSize.width / 2.f,

        padding.bottom +
            innerSize.height / 2.f,
    });

    background->setZOrder(-1);

    this->addChild(background);

    // ! --- ScrollLayer --- !

    auto scrollSize = CCSize{
        innerSize.width - 8.f,
        innerSize.height - 8.f,
    };

    m_scroll =
        ScrollLayer::create(scrollSize);

    m_scroll->setPosition({
        padding.left + 4.f,
        padding.bottom + 4.f,
    });

    m_scroll->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setGap(4.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(
                AxisAlignment::End)
            ->setCrossAxisAlignment(
                AxisAlignment::Center)
            ->setAutoGrowAxis(
                m_scroll->getContentHeight()));

    this->addChild(m_scroll);

    // ! --- Borders --- !

    auto borders = ListBorders::create();

    borders->setSpriteFrames(
        "list-top.png"_spr,
        "list-side.png"_spr,
        2.f);

    borders->setContentSize({
        innerSize.width,
        innerSize.height - 3.f,
    });

    borders->setAnchorPoint({
        0.5f,
        0.5f,
    });

    borders->setPosition({
        padding.left +
            innerSize.width / 2.f,

        padding.bottom +
            innerSize.height / 2.f -
            0.5f,
    });

    borders->updateLayout();

    this->addChild(borders);

    for (auto child :
         CCArrayExt<CCNodeRGBA *>(
             borders->getChildren()))
    {
        child->setColor(
            ccc3(50, 50, 50));
    }

    loadNews();

    return true;
}

void NewsLayer::loadNews(
    bool forceRefresh)
{
    if (!m_scroll)
        return;

    auto cachedResponse =
        loadNewsCache();

    bool const hasCache =
        cachedResponse.has_value();

    if (hasCache)
    {
        setNews(
            cachedResponse->data);
    }
    else
    {
        showMessage(
            "Loading news...");
    }

    if (!forceRefresh &&
        hasCache &&
        isNewsCacheFresh())
    {
        return;
    }

    m_newsTask.spawn(
        api::news::getNews(),
        [this, hasCache](
            web::WebResponse response)
        {
            auto result =
                api::news::
                    parseGetNewsResponse(
                        response);

            if (result.isErr())
            {
                log::error(
                    "Failed to update news: {}",
                    result.unwrapErr());

                // Если кэш уже отображается,
                // ничего в интерфейсе не меняем.
                if (hasCache)
                    return;

                if (response.code() == 429)
                {
                    showMessage(
                        "Too many requests. "
                        "Try again later");
                }
                else
                {
                    showMessage(
                        "Failed to load news");
                }

                return;
            }

            auto newsResponse =
                result.unwrap();

            setNews(
                newsResponse.data);

            saveNewsCache(
                response);
        });
}

void NewsLayer::reload()
{
    loadNews(true);
}

void NewsLayer::showMessage(
    std::string const &message)
{
    if (!m_scroll)
        return;

    auto contentLayer =
        m_scroll->m_contentLayer;

    contentLayer->removeAllChildren();

    auto label =
        CCLabelBMFont::create(
            message.c_str(),
            "bigFont.fnt");

    label->setScale(0.45f);
    label->setOpacity(150);

    contentLayer->addChild(label);
    contentLayer->updateLayout();
}

void NewsLayer::setNews(
    std::vector<NewsItem> const &news)
{
    if (!m_scroll)
        return;

    auto contentLayer =
        m_scroll->m_contentLayer;

    contentLayer->removeAllChildren();

    if (news.empty())
    {
        showMessage("No news yet");
        return;
    }

    auto cardWidth =
        m_scroll->getContentSize().width;

    for (auto const &item : news)
    {
        auto card = NewsCard::create(
            item,
            {
                cardWidth,
                66.f,
            });

        if (card)
            contentLayer->addChild(card);
    }

    contentLayer->updateLayout();
}

std::optional<GetNewsResponse>
NewsLayer::loadNewsCache() const
{
    auto cachedJson =
        Mod::get()
            ->getSavedValue<std::string>(
                "news-cache",
                "");

    if (cachedJson.empty())
        return std::nullopt;

    auto result =
        matjson::parseAs<GetNewsResponse>(
            cachedJson);

    if (result.isErr())
    {
        log::error(
            "Failed to parse news cache: {}",
            result.unwrapErr());

        return std::nullopt;
    }

    return result.unwrap();
}

void NewsLayer::saveNewsCache(
    web::WebResponse const &response) const
{
    auto bodyResult =
        response.string();

    if (bodyResult.isErr())
    {
        log::error(
            "Failed to read news response "
            "for cache: {}",
            bodyResult.unwrapErr());

        return;
    }

    Mod::get()
        ->setSavedValue<std::string>(
            "news-cache",
            bodyResult.unwrap());

    Mod::get()
        ->setSavedValue<std::int64_t>(
            "news-cache-updated-at",
            static_cast<std::int64_t>(
                std::time(nullptr)));

    auto saveResult =
        Mod::get()->saveData();

    if (saveResult.isErr())
    {
        log::error(
            "Failed to save news cache: {}",
            saveResult.unwrapErr());
    }
}

bool NewsLayer::isNewsCacheFresh() const
{
    auto cachedAt =
        Mod::get()
            ->getSavedValue<std::int64_t>(
                "news-cache-updated-at",
                0);

    if (cachedAt <= 0)
        return false;

    auto now =
        static_cast<std::int64_t>(
            std::time(nullptr));

    auto age =
        now - cachedAt;

    return age >= 0 &&
           age < CACHE_TTL_SECONDS;
}