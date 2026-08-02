#include "NewsCard.hpp"

#include "../../../../ui/RectNode.hpp"

#include <algorithm>
#include <ctime>

NewsCard *NewsCard::create(
    NewsItem const &news,
    CCSize const &size)
{
    auto ret = new NewsCard();

    if (ret && ret->init(news, size))
    {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

std::string NewsCard::formatRelativeTime(
    std::time_t timestamp)
{
    if (timestamp <= 0)
        return "Unknown date";

    auto seconds = std::max<std::time_t>(
        std::time(nullptr) - timestamp,
        0);

    if (seconds < 60)
        return "Just now";

    auto minutes = seconds / 60;

    if (minutes < 60)
    {
        return fmt::format(
            "{} minute{} ago",
            minutes,
            minutes == 1 ? "" : "s");
    }

    auto hours = minutes / 60;

    if (hours < 24)
    {
        return fmt::format(
            "{} hour{} ago",
            hours,
            hours == 1 ? "" : "s");
    }

    auto days = hours / 24;

    return fmt::format(
        "{} day{} ago",
        days,
        days == 1 ? "" : "s");
}

ccColor4B NewsCard::getAccentColor(
    NewsType type)
{
    switch (type)
    {
    case NewsType::StartPosPublished:
        return {80, 220, 70, 255};

    case NewsType::ModUpdate:
        return {70, 150, 255, 255};

    case NewsType::Warning:
        return {255, 110, 60, 255};

    case NewsType::Announcement:
        return {255, 190, 40, 255};

    default:
        return {130, 130, 130, 255};
    }
}

std::string NewsCard::getTypeName(
    NewsType type)
{
    switch (type)
    {
    case NewsType::StartPosPublished:
        return "STARTPOS";

    case NewsType::ModUpdate:
        return "UPDATE";

    case NewsType::Warning:
        return "WARNING";

    case NewsType::Announcement:
        return "NEWS";

    default:
        return "UNKNOWN";
    }
}

void NewsCard::onAction(
    CCObject *sender)
{
    auto button =
        static_cast<
            CCMenuItemSpriteExtra *>(
            sender);

    if (!button)
        return;

    auto actionIndex =
        button->getTag();

    if (
        actionIndex < 0 ||
        actionIndex >=
            static_cast<int>(
                m_news.actions.size()))
    {
        return;
    }

    auto const &action =
        m_news.actions[actionIndex];

    switch (action.type)
    {
    case NewsActionType::CopyText:
    {
        auto copied =
            geode::utils::clipboard::write(
                action.value);

        Notification::create(
            copied
                ? "Copied to clipboard"
                : "Failed to copy text",
            copied
                ? NotificationIcon::Success
                : NotificationIcon::Error)
            ->show();

        break;
    }

    case NewsActionType::OpenURL:
    {
        geode::utils::web::
            openLinkInBrowser(
                action.value);

        break;
    }

    case NewsActionType::OpenLevel:
    {
        auto search =
            GJSearchObject::create(
                SearchType::Search,
                action.value);

        auto scene =
            LevelBrowserLayer::scene(
                search);

        CCDirector::sharedDirector()
            ->pushScene(
                CCTransitionFade::create(
                    0.3f,
                    scene));

        break;
    }

    default:
        Notification::create(
            "Unsupported news action",
            NotificationIcon::Error)
            ->show();

        break;
    }
}

bool NewsCard::init(
    NewsItem const &news,
    CCSize const &size)
{
    if (!CCLayer::init())
        return false;

    m_news = news;
    m_size = size;

    this->setContentSize(size);
    this->ignoreAnchorPointForPosition(false);
    this->setAnchorPoint({0.5f, 0.5f});

    auto accentColor =
        getAccentColor(news.type);

    float const leftPadding = 16.f;
    float const rightPadding = 12.f;

    // ! --- Calculate actions width --- !

    float actionsWidth = 0.f;

    for (auto const &action : news.actions)
    {
        auto buttonWidth = std::max(
            58.f,
            static_cast<float>(
                action.label.size()) *
                    5.f +
                16.f);

        actionsWidth += buttonWidth + 6.f;
    }

    if (actionsWidth > 0.f)
        actionsWidth -= 6.f;

    auto contentRight =
        size.width -
        rightPadding -
        (actionsWidth > 0.f
             ? actionsWidth + 12.f
             : 0.f);

    auto contentWidth =
        std::max(
            contentRight - leftPadding,
            30.f);

    // ! --- Accent border --- !

    auto border = RectNode::create(
        size,
        ccc4FFromccc4B({
            accentColor.r,
            accentColor.g,
            accentColor.b,
            170,
        }),
        7.f);

    border->ignoreAnchorPointForPosition(false);
    border->setAnchorPoint({0.5f, 0.5f});

    border->setPosition({
        size.width / 2.f,
        size.height / 2.f,
    });

    this->addChild(border);

    // ! --- Card background --- !

    auto background = RectNode::create(
        {
            size.width - 2.f,
            size.height - 2.f,
        },
        ccc4FFromccc4B({
            24,
            30,
            25,
            255,
        }),
        6.f);

    background->ignoreAnchorPointForPosition(false);
    background->setAnchorPoint({0.5f, 0.5f});

    background->setPosition({
        size.width / 2.f,
        size.height / 2.f,
    });

    this->addChild(background);

    // ! --- Left accent --- !

    auto accent = RectNode::create(
        {
            4.f,
            size.height - 12.f,
        },
        ccc4FFromccc4B(accentColor),
        2.f);

    accent->ignoreAnchorPointForPosition(false);
    accent->setAnchorPoint({0.f, 0.5f});

    accent->setPosition({
        7.f,
        size.height / 2.f,
    });

    this->addChild(accent);

    // ! --- Type badge --- !

    auto badge = RectNode::create(
        {52.f, 14.f},
        ccc4FFromccc4B({
            accentColor.r,
            accentColor.g,
            accentColor.b,
            90,
        }),
        4.f);

    badge->ignoreAnchorPointForPosition(false);
    badge->setAnchorPoint({0.f, 0.5f});

    badge->setPosition({
        leftPadding,
        size.height - 11.f,
    });

    this->addChild(badge);

    auto typeLabel =
        CCLabelBMFont::create(
            getTypeName(news.type).c_str(),
            "bigFont.fnt");

    typeLabel->setScale(0.25f);
    typeLabel->setAnchorPoint({0.5f, 0.5f});

    typeLabel->setPosition({
        badge->getContentWidth() / 2.f,
        badge->getContentHeight() / 2.f,
    });

    badge->addChild(typeLabel);

    // ! --- Pinned label --- !

    if (news.pinned)
    {
        auto pinnedLabel =
            CCLabelBMFont::create(
                "PINNED",
                "goldFont.fnt");

        pinnedLabel->setScale(0.25f);
        pinnedLabel->setAnchorPoint({0.f, 0.5f});

        pinnedLabel->setPosition({
            leftPadding +
                badge->getContentWidth() +
                7.f,

            size.height - 11.f,
        });

        this->addChild(pinnedLabel);
    }

    // ! --- Published date --- !

    auto publishedLabel =
        CCLabelBMFont::create(
            formatRelativeTime(
                news.publishedAt)
                .c_str(),
            "chatFont.fnt");

    publishedLabel->setScale(0.32f);
    publishedLabel->setOpacity(140);
    publishedLabel->setAnchorPoint({1.f, 0.5f});

    publishedLabel->setPosition({
        size.width - rightPadding,
        size.height - 11.f,
    });

    this->addChild(publishedLabel);

    // ! --- Title --- !

    auto title =
        CCLabelBMFont::create(
            news.title.c_str(),
            "bigFont.fnt");

    title->setAnchorPoint({0.f, 0.5f});
    title->setPosition({leftPadding, 37.f});

    title->limitLabelWidth(
        contentWidth,
        0.41f,
        0.29f);

    this->addChild(title);

    // ! --- Description --- !

    auto description =
        CCLabelBMFont::create(
            news.description.c_str(),
            "chatFont.fnt");

    description->setAnchorPoint({0.f, 0.5f});
    description->setPosition({leftPadding, 18.f});
    description->setOpacity(210);

    description->limitLabelWidth(
        contentWidth,
        0.47f,
        0.32f);

    this->addChild(description);

    // ! --- Actions --- !

    if (!news.actions.empty())
    {
        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});

        auto buttonX =
            size.width - rightPadding;

        for (
            auto actionIndex =
                static_cast<int>(
                    news.actions.size()) -
                1;

            actionIndex >= 0;
            --actionIndex)
        {
            auto const &action =
                news.actions[actionIndex];

            auto buttonWidth = std::max(
                58.f,
                static_cast<float>(
                    action.label.size()) *
                        5.f +
                    16.f);

            auto buttonBackground =
                RectNode::create(
                    {buttonWidth, 22.f},
                    ccc4FFromccc4B(
                        accentColor),
                    5.f);

            buttonBackground
                ->ignoreAnchorPointForPosition(
                    false);

            auto buttonLabel =
                CCLabelBMFont::create(
                    action.label.c_str(),
                    "bigFont.fnt");

            buttonLabel->setScale(0.32f);
            buttonLabel->setAnchorPoint({0.5f, 0.5f});

            buttonLabel->setPosition({
                buttonWidth / 2.f,
                11.f,
            });

            buttonBackground->addChild(
                buttonLabel);

            auto button =
                CCMenuItemSpriteExtra::create(
                    buttonBackground,
                    this,
                    menu_selector(
                        NewsCard::onAction));

            button->setTag(actionIndex);

            button->setPosition({
                buttonX - buttonWidth / 2.f,
                size.height / 2.f - 3.f,
            });

            buttonX -=
                buttonWidth + 6.f;

            menu->addChild(button);
        }

        this->addChild(menu);
    }

    return true;
}