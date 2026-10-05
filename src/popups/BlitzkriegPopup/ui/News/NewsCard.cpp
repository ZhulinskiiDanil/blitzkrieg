#include "NewsCard.hpp"

#include "../../../../ui/RectNode.hpp"
#include "../../../../ui/ScrollClip.hpp"
#include "../../../../utils/ui/fitLabelWidth.hpp"

#include <algorithm>
#include <ctime>

namespace
{
    // Neutral card colors, the news type is shown only by small accents
    constexpr ccColor3B CARD_BACKGROUND = {36, 36, 36};
    constexpr ccColor3B CARD_BORDER = {58, 58, 58};
    constexpr ccColor3B PINNED_COLOR = {225, 190, 110};

    ccColor4F toColor4F(ccColor3B color)
    {
        return ccc4FFromccc4B({color.r, color.g, color.b, 255});
    }

    // ! --- Chips and buttons, the same as in the Backups tab --- !

    constexpr float CHIP_HEIGHT = 11.f;
    constexpr float CHIP_LABEL_SCALE = .22f;
    constexpr float BUTTON_HEIGHT = 16.f;
    constexpr float BUTTON_LABEL_SCALE = .28f;
    constexpr float BUTTON_MIN_WIDTH = 50.f;
    constexpr ccColor4B BUTTON_COLOR = {55, 55, 55, 255};

    // Colored text on a faint fill of the same color, its left edge is the origin
    CCNode *createChip(
        std::string const &text,
        ccColor3B color)
    {
        auto label =
            CCLabelBMFont::create(
                text.c_str(),
                "bigFont.fnt");

        label->setScale(CHIP_LABEL_SCALE);
        label->setColor(color);

        CCSize const size{
            label->getScaledContentWidth() + 10.f,
            CHIP_HEIGHT,
        };

        auto fill = toColor4F(color);
        fill.a = .15f;

        auto chip = CCNode::create();
        chip->setContentSize(size);
        chip->setAnchorPoint({0.f, 0.5f});

        chip->addChild(RectNode::create(
            size,
            premultiplyAlpha(fill),
            size.height / 2.f));

        label->setPosition(size / 2.f);
        chip->addChild(label);

        return chip;
    }

    // Wide enough for the label, never narrower than the Backups buttons
    float getButtonWidth(
        std::string const &text)
    {
        auto label =
            CCLabelBMFont::create(
                text.c_str(),
                "bigFont.fnt");

        return std::max(
            BUTTON_MIN_WIDTH,
            label->getContentWidth() *
                    BUTTON_LABEL_SCALE +
                16.f);
    }
}

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
    // Muted tones: readable on the dark card, but not glowing
    case NewsType::StartPosPublished:
        return {120, 190, 125, 255};

    case NewsType::ModUpdate:
        return {120, 160, 220, 255};

    case NewsType::Warning:
        return {220, 140, 105, 255};

    case NewsType::Announcement:
        return {215, 185, 115, 255};

    default:
        return {140, 140, 140, 255};
    }
}

std::string NewsCard::getTypeName(
    NewsType type)
{
    switch (type)
    {
    case NewsType::StartPosPublished:
        return "Start pos";

    case NewsType::ModUpdate:
        return "Update";

    case NewsType::Warning:
        return "Warning";

    case NewsType::Announcement:
        return "News";

    default:
        return "Unknown";
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

    this->ignoreAnchorPointForPosition(false);
    this->setAnchorPoint({0.5f, 0.5f});

    auto accentColor =
        getAccentColor(news.type);

    ccColor3B const accent3B{
        accentColor.r,
        accentColor.g,
        accentColor.b,
    };

    float const leftPadding = 16.f;
    float const rightPadding = 12.f;

    // ! --- Layout offsets --- !

    float const topStripOffset = 14.f;
    float const titleTopOffset = 31.f;
    float const descriptionTopOffset = 40.f;
    float const descriptionBottomPadding = 8.f;
    // Buttons are on the title line
    float const actionsTopOffset = 31.f;

    // ! --- Calculate actions width --- !

    float actionsWidth = 0.f;

    for (auto const &action : news.actions)
        actionsWidth += getButtonWidth(action.label) + 6.f;

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

    // ! --- Description label --- !

    float const descriptionScale = 0.55f;

    auto description =
        Label::create(
            news.description,
            "chatFont.fnt");

    description->setOpacity(210);
    description->setScale(descriptionScale);

    description->setMaxWidth(
        contentWidth / descriptionScale);

    description->setBreakWords(true);

    auto descriptionHeight =
        description->getScaledContentSize()
            .height;

    // ! --- Card height --- !

    auto cardHeight =
        std::max(
            size.height,
            descriptionTopOffset +
                descriptionHeight +
                descriptionBottomPadding);

    CCSize const cardSize{
        size.width,
        cardHeight,
    };

    m_size = cardSize;

    this->setContentSize(cardSize);

    // ! --- Border --- !
    // Inset on the top, left and right, see SCROLL_CLIP_INSET

    CCSize const borderSize{
        cardSize.width - SCROLL_CLIP_INSET * 2.f,
        cardHeight - SCROLL_CLIP_INSET,
    };

    auto border = RectNode::create(
        borderSize,
        toColor4F(CARD_BORDER),
        7.f);

    border->ignoreAnchorPointForPosition(false);
    border->setAnchorPoint({0.5f, 0.f});

    border->setPosition({
        cardSize.width / 2.f,
        0.f,
    });

    this->addChild(border);

    // ! --- Card background --- !

    auto background = RectNode::create(
        {
            borderSize.width - 2.f,
            borderSize.height - 2.f,
        },
        toColor4F(CARD_BACKGROUND),
        6.f);

    background->ignoreAnchorPointForPosition(false);
    background->setAnchorPoint({0.5f, 0.f});

    background->setPosition({
        cardSize.width / 2.f,
        1.f,
    });

    this->addChild(background);

    // ! --- Left accent --- !

    auto accent = RectNode::create(
        {
            3.f,
            cardHeight - 14.f,
        },
        ccc4FFromccc4B(accentColor),
        1.5f);

    accent->ignoreAnchorPointForPosition(false);
    accent->setAnchorPoint({0.f, 0.5f});

    accent->setPosition({
        7.f,
        cardHeight / 2.f,
    });

    this->addChild(accent);

    // ! --- Type chip --- !

    auto badge = createChip(
        getTypeName(news.type),
        accent3B);

    badge->setPosition({
        leftPadding,
        cardHeight - topStripOffset,
    });

    this->addChild(badge);

    // ! --- Pin icon, as tall as the chip --- !

    if (news.pinned)
    {
        if (auto pin = CCSprite::createWithSpriteFrameName("pin.png"_spr))
        {
            pin->setScale(
                CHIP_HEIGHT /
                std::max(
                    pin->getContentWidth(),
                    pin->getContentHeight()));

            pin->setColor(PINNED_COLOR);
            pin->setAnchorPoint({0.f, 0.5f});

            pin->setPosition({
                leftPadding +
                    badge->getContentWidth() +
                    5.f,

                cardHeight - topStripOffset,
            });

            this->addChild(pin);
        }
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
        cardSize.width - rightPadding,
        cardHeight - topStripOffset,
    });

    this->addChild(publishedLabel);

    // ! --- Title --- !

    auto title =
        CCLabelBMFont::create(
            "",
            "bigFont.fnt");

    title->setAnchorPoint({0.f, 0.5f});

    title->setPosition({
        leftPadding,
        cardHeight - titleTopOffset,
    });

    // Shrinks first, then cuts with "..." so it never runs under the actions
    fitLabelWidth(
        title,
        news.title,
        contentWidth,
        0.41f,
        0.29f);

    this->addChild(title);

    // ! --- Description --- !

    description->setAnchorPoint({0.f, 1.f});

    description->setPosition({
        leftPadding,
        cardHeight - descriptionTopOffset,
    });

    this->addChild(description);

    // ! --- Actions --- !

    if (!news.actions.empty())
    {
        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});

        auto buttonX =
            cardSize.width - rightPadding;

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

            auto buttonWidth =
                getButtonWidth(action.label);

            CCSize const buttonSize{
                buttonWidth,
                BUTTON_HEIGHT,
            };

            auto buttonContent = CCNode::create();
            buttonContent->setContentSize(buttonSize);

            buttonContent->addChild(RectNode::create(
                buttonSize,
                ccc4FFromccc4B(BUTTON_COLOR),
                buttonSize.height / 2.f));

            // Colored text on a gray pill, like Restore
            auto buttonLabel =
                CCLabelBMFont::create(
                    action.label.c_str(),
                    "bigFont.fnt");

            buttonLabel->limitLabelWidth(
                buttonWidth - 10.f,
                BUTTON_LABEL_SCALE,
                .1f);

            buttonLabel->setColor(accent3B);
            buttonLabel->setPosition(buttonSize / 2.f);

            buttonContent->addChild(buttonLabel);

            auto button =
                CCMenuItemSpriteExtra::create(
                    buttonContent,
                    this,
                    menu_selector(
                        NewsCard::onAction));

            button->m_scaleMultiplier = 1.05f;
            button->setTag(actionIndex);

            button->setPosition({
                buttonX - buttonWidth / 2.f,
                cardHeight - actionsTopOffset,
            });

            buttonX -=
                buttonWidth + 6.f;

            menu->addChild(button);
        }

        this->addChild(menu);
    }

    return true;
}