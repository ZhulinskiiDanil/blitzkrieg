#include "StageRangeCell.hpp"

#include <algorithm>
#include <cmath>
#include <ctime>

namespace
{
    std::string formatCount(
        int count,
        std::string_view singular,
        std::string_view plural)
    {
        return fmt::format(
            "{} <small>{}</small>",
            count,
            count == 1 ? singular : plural);
    }

    std::string formatPercentage(float value)
    {
        const int hundredths =
            static_cast<int>(std::round(value * 100.f));

        const int whole = hundredths / 100;
        const int decimal = std::abs(hundredths % 100);

        return fmt::format(
            "{}<small>.{:02d}%</small>",
            whole,
            decimal);
    }

    std::string formatRun(float from, float to)
    {
        return fmt::format(
            "{} - {}",
            formatPercentage(from),
            formatPercentage(to));
    }

    float runLength(float from, float to)
    {
        return std::abs(to - from);
    }

    // I'm so sorry
    std::string formatCompletedAt(std::time_t timestamp)
    {
        if (timestamp <= 0)
            return {};

        auto timeInfo = geode::localtime(timestamp);

        return fmt::format(
            "{:%Y-%m-%d %H:%M}",
            timeInfo);
    }

    std::string compactNote(std::string note)
    {
        std::replace(note.begin(), note.end(), '\n', ' ');
        std::replace(note.begin(), note.end(), '\r', ' ');

        constexpr std::size_t maxLength = 42;

        if (note.size() > maxLength)
            note = note.substr(0, maxLength - 1) + "…";

        return note;
    }
}

StageRangeCell *StageRangeCell::create(
    Range *range,
    GJGameLevel *level,
    const CCSize &cellSize)
{
    auto *ret = new StageRangeCell();

    if (ret && ret->init(range, level, cellSize))
    {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool StageRangeCell::init(
    Range *range,
    GJGameLevel *level,
    const CCSize &cellSize)
{
    if (!CCLayer::init() || !range)
        return false;

    this->setContentSize(cellSize);

    const bool expandedByDefault =
        Mod::get()->getSettingValue<bool>(
            "expand-progress-by-default");

    m_size = cellSize;
    m_range = range;
    m_from = range->from;
    m_to = range->to;
    m_id = range->id;
    m_checked = range->checked;
    m_isExpanded = expandedByDefault;
    m_level = level;

    if (auto *profile =
            GlobalStore::get()->getProfileByLevel(m_level))
    {
        const auto currentRange =
            GlobalStore::get()->getCurrentRange(profile->id);

        if (!currentRange.id.empty())
            m_isCurrent = currentRange.id == m_range->id;
    }

    m_content = CCLayer::create();
    m_content->setZOrder(1);
    m_content->setContentSize(cellSize);
    m_content->setPosition({0, cellSize.height});
    m_content->setAnchorPoint({0, 1});
    m_content->setLayout(
        ColumnLayout::create()
            ->setGap(0)
            ->setAxisAlignment(AxisAlignment::End)
            ->setCrossAxisAlignment(AxisAlignment::Center)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setAutoGrowAxis(true));
    m_content->setID("progress-cell-content"_spr);
    this->addChild(m_content);

    m_head = CCLayer::create();
    m_head->setContentSize(cellSize);
    m_head->setID("progress-cell-head"_spr);
    m_content->addChild(m_head);

    updateTexture();

    const std::string rangeText = fmt::format(
        "{:.0f}<small>.{:02.0f}%</small> - {:.0f}<small>.{:02.0f}%</small>",
        std::floor(m_range->from),
        std::round(
            (m_range->from - std::floor(m_range->from)) * 100),
        std::floor(m_range->to),
        std::round(
            (m_range->to - std::floor(m_range->to)) * 100));

    m_rangeLabel =
        UILabel::create(rangeText, "gjFont17.fnt", .4f);
    m_rangeLabel->setPosition(
        {25.f, cellSize.height / 2});
    m_rangeLabel->setAnchorPoint({0.f, .5f});
    m_head->addChild(m_rangeLabel, 3);

    m_statusLabel =
        CCLabelBMFont::create("", "bigFont.fnt");
    m_statusLabel->setScale(.24f);
    m_statusLabel->setAnchorPoint({1.f, .5f});
    m_statusLabel->setPosition(
        {cellSize.width - 34.f, cellSize.height / 2});
    m_statusLabel->setOpacity(210);
    m_head->addChild(m_statusLabel, 4);

    updateTextColors();
    updateStatusBadge();

    auto toggleOn =
        CCSprite::createWithSpriteFrameName(
            "GJ_checkOn_001.png");
    auto toggleOff =
        CCSprite::createWithSpriteFrameName(
            "GJ_checkOff_001.png");

    m_checkbox = CCMenuItemToggler::create(
        toggleOn,
        toggleOff,
        this,
        menu_selector(StageRangeCell::onToggle));

    m_checkbox->setAnchorPoint({0.5f, 0.5f});
    m_checkbox->setCascadeColorEnabled(true);
    m_checkbox->toggle(!m_checked);

    auto checkboxMenu =
        CCMenu::createWithItem(m_checkbox);

    checkboxMenu->setScale(0.5f);
    checkboxMenu->setAnchorPoint({0.5f, 0.f});
    checkboxMenu->setContentSize({32.f, 32.f});
    checkboxMenu->setPosition(
        {5.f, cellSize.height / 2});

    m_head->addChild(checkboxMenu);

    updateExpandButton();

    m_table = MetaTable::create(
        buildMetaData(),
        m_head->getContentWidth(),
        {5, 5, 5, 5});

    m_content->addChild(m_table);

    updateMetaContent();
    updateExpandButton();
    updateLayoutWrapper(true);

    return true;
}

std::vector<MetaData>
StageRangeCell::buildMetaData() const
{
    if (!m_range)
        return {};

    const bool isCompleted =
        m_range->checked;

    const bool isAutoClosed =
        isCompleted &&
        m_range->automaticallyClosed;

    const bool isManualClosed =
        isCompleted &&
        !isAutoClosed &&
        m_range->attemptsToComplete <= 0;

    const bool hasFirstClear =
        m_range->firstRunFrom >= 0.f &&
        m_range->firstRunTo > 0.f;

    const bool hasBestRun =
        m_range->bestRunFrom >= 0.f &&
        m_range->bestRunTo > 0.f;

    const std::string activity =
        fmt::format(
            "{} / {}",
            formatCount(
                m_range->attempts,
                "attempt",
                "attempts"),
            formatTimePlayed(
                m_range->timePlayed));

    const std::string firstClear =
        hasFirstClear
            ? formatRun(
                  m_range->firstRunFrom,
                  m_range->firstRunTo)
            : "<small>None</small>";

    std::string bestRun =
        hasBestRun
            ? formatRun(
                  m_range->bestRunFrom,
                  m_range->bestRunTo)
            : "<small>None</small>";

    if (hasBestRun)
    {
        bestRun += fmt::format(
            " <small>+{:.2f}%</small>",
            runLength(
                m_range->bestRunFrom,
                m_range->bestRunTo));
    }

    std::vector<MetaData> data;

    data.push_back({
        "Activity:",
        activity,
    });

    data.push_back({
        "First Clear:",
        firstClear,
    });

    data.push_back({
        "Best Run:",
        bestRun,
    });

    if (isManualClosed)
    {
        data.push_back({
            "Status:",
            "<small>Manually closed</small>",
        });
    }
    else if (isAutoClosed)
    {
        data.push_back({
            "Status:",
            "<small>Automatically closed</small>",
        });
    }
    else if (!isCompleted)
    {
        data.push_back({
            "Status:",
            "<small>Not completed</small>",
        });
    }
    else
    {
        data.push_back({
            "Clear Attempts:",
            fmt::format(
                "<small>in</small> {}",
                formatCount(
                    m_range->attemptsToComplete,
                    "attempt",
                    "attempts")),
        });

        data.push_back({
            "Passes:",
            formatCount(
                m_range->completionCounter,
                "pass",
                "passes"),
        });

        if (m_range->attempts > 0)
        {
            const float passRate =
                100.f *
                static_cast<float>(
                    m_range->completionCounter) /
                static_cast<float>(
                    m_range->attempts);

            data.push_back({
                "Pass Rate:",
                fmt::format(
                    "{:.1f}<small>%</small> "
                    "<small>({}/{})</small>",
                    passRate,
                    m_range->completionCounter,
                    m_range->attempts),
            });
        }
    }

    if (const auto completedAt =
            formatCompletedAt(
                m_range->completedAt);
        !completedAt.empty())
    {
        data.push_back({
            "Completed:",
            fmt::format(
                "<small>{}</small>",
                completedAt),
        });
    }

    if (!m_range->note.empty())
    {
        data.push_back({
            "Note:",
            fmt::format(
                "<small>{}</small>",
                compactNote(
                    m_range->note)),
        });
    }

    std::reverse(
        data.begin(),
        data.end());

    return data;
}

void StageRangeCell::updateMetaTable()
{
    if (!m_table)
        return;

    m_table->setData(buildMetaData());
    updateMetaContent();
}

void StageRangeCell::updateLayoutWrapper(
    bool isInitialRender)
{
    const bool animDisabled =
        Mod::get()->getSettingValue<bool>(
            "disable-progress-expansion-animation") ||
        isInitialRender;

    this->stopAllActions();

    const auto oldContentSize =
        m_content->getContentSize();

    m_content->updateLayout();

    const auto newContentSize =
        m_content->getContentSize();

    if (!animDisabled)
        m_content->setContentSize(oldContentSize);

    if (m_table && !animDisabled)
    {
        m_table->setVisible(
            isInitialRender
                ? m_isExpanded
                : !m_isExpanded);
    }

    auto resizeTo = CCEaseInOut::create(
        CCResizeTo::create(
            0.26f,
            newContentSize.width,
            newContentSize.height),
        2);

    auto actionFloat = CCActionFloat::create(
        0.26f,
        0,
        1,
        [this, newContentSize](float)
        {
            m_head->setPosition(
                this->getContentWidth() / 2,
                this->getContentHeight() -
                    m_head->getContentHeight() / 2);

            m_content->setPosition(
                {0, this->getContentHeight()});

            m_content->setContentSize(
                this->getContentSize());

            updateTexture();
            UpdateScrollLayoutEvent().send();
        });

    auto delay = CCDelayTime::create(
        !m_isExpanded ? 0.26f : 0);

    auto spawnResize =
        CCSpawn::createWithTwoActions(
            resizeTo,
            actionFloat);

    CCFiniteTimeAction *spawnDelay = nullptr;

    if (!m_isExpanded)
    {
        spawnDelay = CCSpawn::createWithTwoActions(
            delay,
            actionFloat);
    }
    else
    {
        spawnDelay = delay;
    }

    auto animateTable = CCCallFunc::create(
        this,
        callfunc_selector(
            StageRangeCell::onFinishExpandAnimation));

    if (animDisabled)
    {
        this->setContentSize(newContentSize);
        m_content->setPosition(
            {0, this->getContentHeight()});
        m_content->setContentSize(newContentSize);
        m_content->updateLayout();

        updateTexture();
        UpdateScrollLayoutEvent().send();
    }
    else
    {
        this->runAction(
            CCSequence::create(
                animateTable,
                spawnDelay,
                spawnResize,
                nullptr));
    }
}

void StageRangeCell::onFinishExpandAnimation()
{
    if (!m_table)
        return;

    if (m_isExpanded)
        m_table->setVisible(true);

    const bool animDisabled =
        Mod::get()->getSettingValue<bool>(
            "disable-progress-expansion-animation");

    m_table->stopAllActions();

    auto scaleTo = CCSequence::createWithTwoActions(
        CCDelayTime::create(
            m_isExpanded ? .26f : 0),
        CCScaleTo::create(
            0.15f,
            m_isExpanded ? 1 : 0));

    auto endFunc = CCCallFunc::create(
        this,
        callfunc_selector(
            StageRangeCell::onFinishTableAnimation));

    if (!animDisabled)
    {
        m_table->setScale(
            !m_isExpanded ? 1 : 0);

        m_table->runAction(
            CCSequence::createWithTwoActions(
                CCEaseInOut::create(scaleTo, 2),
                endFunc));
    }
}

void StageRangeCell::onFinishTableAnimation()
{
    if (m_table)
        m_table->setVisible(m_isExpanded);
}

void StageRangeCell::updateTexture()
{
    if (!m_head || !m_range)
        return;

    if (m_background)
        m_background->removeFromParentAndCleanup(true);

    if (m_lineBg)
        m_lineBg->removeFromParentAndCleanup(true);

    if (m_bestRunLineBg)
        m_bestRunLineBg->removeFromParentAndCleanup(true);

    if (m_midBg)
        m_midBg->removeFromParentAndCleanup(true);

    if (m_mid)
        m_mid->removeFromParentAndCleanup(true);

    constexpr float lineBgPaddingX = 5.f;
    constexpr float lineBgH = 2.f;

    const float lineBgW =
        std::max(
            0.f,
            m_size.width - lineBgPaddingX * 2);

    const auto clampPercent = [](float value)
    {
        return std::clamp(value, 0.f, 100.f);
    };

    const float bestFrom =
        clampPercent(m_range->bestRunFrom);
    const float bestTo =
        clampPercent(m_range->bestRunTo);

    const float targetFrom =
        clampPercent(m_from);
    const float targetTo =
        clampPercent(m_to);

    const float bestRunLineW =
        lineBgW *
        (std::max(0.f, bestTo - bestFrom) / 100.f);

    const float bestRunLineX =
        lineBgPaddingX +
        lineBgW * (bestFrom / 100.f);

    const float targetLineW =
        lineBgW *
        (std::max(0.f, targetTo - targetFrom) / 100.f);

    const float targetLineX =
        lineBgPaddingX +
        lineBgW * (targetFrom / 100.f);

    const auto lineBgColor =
        m_disabled
            ? ccc4FFromccc4B({56, 38, 38, 255})
        : m_checked
            ? ccc4FFromccc4B({48, 66, 45, 255})
        : m_isCurrent
            ? ccc4FFromccc4B({61, 58, 42, 255})
            : ccc4FFromccc4B({54, 54, 54, 255});

    const auto targetBgColor =
        m_disabled
            ? ccc4FFromccc4B({87, 54, 54, 255})
        : m_checked
            ? ccc4FFromccc4B({70, 117, 61, 255})
        : m_isCurrent
            ? ccc4FFromccc4B({97, 90, 52, 255})
            : ccc4FFromccc4B({84, 84, 84, 255});

    const auto targetColor =
        m_disabled
            ? ccc4FFromccc4B({218, 80, 80, 255})
        : m_checked
            ? ccc4FFromccc4B({98, 240, 70, 255})
        : m_isCurrent
            ? ccc4FFromccc4B({196, 175, 72, 255})
            : ccc4FFromccc4B({201, 201, 201, 255});

    const auto bestRunColor =
        targetBgColor;

    m_lineBg = RectNode::create(
        {lineBgW, lineBgH},
        lineBgColor,
        1);

    m_bestRunLineBg = RectNode::create(
        {bestRunLineW, lineBgH},
        bestRunColor,
        1);

    m_midBg = RectNode::create(
        {targetLineW, lineBgH + 2},
        targetBgColor,
        lineBgH / 2 + 2);

    m_mid = RectNode::create(
        {std::max(0.f, targetLineW - 2.f), lineBgH},
        targetColor,
        lineBgH / 2);

    m_lineBg->setPosition(
        {lineBgPaddingX, lineBgH - 2});

    m_bestRunLineBg->setPosition(
        {bestRunLineX, lineBgH - 2});

    m_midBg->setPosition(
        {targetLineX, lineBgH - 3});

    m_mid->setPosition(
        {targetLineX + 1, lineBgH - 2});

    m_head->addChild(m_lineBg, 1);

    if (
        m_range->bestRunFrom >= 0 &&
        m_range->bestRunTo > 0 &&
        bestRunLineW > 0)
    {
        m_head->addChild(m_bestRunLineBg, 2);
    }

    m_head->addChild(m_midBg, 3);
    m_head->addChild(m_mid, 4);

    const auto contentSize =
        m_content->getContentSize();

    const auto bgSpr =
        m_disabled
            ? "range-disabled-bg.png"_spr
        : m_checked
            ? "range-completed-bg.png"_spr
        : m_isCurrent
            ? "range-current-bg.png"_spr
            : "range-default-bg.png"_spr;

    m_background =
        CCScale9Sprite::create(bgSpr);

    m_background->setContentSize(contentSize);
    m_background->setPosition(
        {contentSize.width / 2,
         contentSize.height / 2});
    m_background->setZOrder(0);

    this->addChild(m_background);
}

void StageRangeCell::updateExpandButton()
{
    const auto cellSize =
        m_head->getContentSize();

    if (m_expandBtnMenu)
    {
        m_expandBtnMenu->stopAllActions();

        m_expandBtnMenu->setPosition(
            {cellSize.width - 5.f,
             cellSize.height / 2});

        if (auto *sprite =
                typeinfo_cast<CCMenuItemSpriteExtra *>(
                    m_expandBtnMenu->getChildByIndex(0)))
        {
            auto rotateTo = CCRotateTo::create(
                0.15f,
                m_isExpanded ? 0 : 180);

            sprite->runAction(
                CCEaseInOut::create(
                    rotateTo,
                    2));
        }

        return;
    }

    m_expandBtnMenu = CCMenu::create();
    m_expandBtnMenu->setAnchorPoint({1.f, 0.5f});
    m_expandBtnMenu->setPosition(
        {cellSize.width - 5.f,
         cellSize.height / 2});

    m_expandBtnMenu->setLayout(
        RowLayout::create()
            ->setGap(2.5f)
            ->setAutoScale(false)
            ->setAutoGrowAxis(true)
            ->setAxisAlignment(
                AxisAlignment::End)
            ->setCrossAxisAlignment(
                AxisAlignment::Center));

    m_head->addChild(m_expandBtnMenu);

    auto spr =
        CCSprite::createWithSpriteFrameName(
            "purple-chevron-up-btn.png"_spr);

    const auto expandBtn =
        CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(
                StageRangeCell::onExpand));

    expandBtn->ignoreAnchorPointForPosition(true);
    expandBtn->setRotation(
        m_isExpanded ? 0 : 180);
    expandBtn->setScale(.5f);
    expandBtn->m_baseScale =
        expandBtn->getScale();

    m_expandBtnMenu->addChild(expandBtn);
    m_expandBtnMenu->updateLayout();
}

void StageRangeCell::updateTextColors()
{
    if (!m_rangeLabel)
        return;

    if (m_disabled)
    {
        m_rangeLabel->setVariant(
            UILabel::Variant::Red);
    }
    else if (m_checked)
    {
        m_rangeLabel->setVariant(
            UILabel::Variant::Green);
    }
    else
    {
        m_rangeLabel->setVariant(
            UILabel::Variant::Orange);
    }
}

void StageRangeCell::updateStatusBadge()
{
    if (!m_statusLabel || !m_range)
        return;

    std::string text;
    ccColor3B color = {255, 255, 255};

    if (m_disabled)
    {
        text = "LOCKED";
        color = {255, 110, 110};
    }
    else if (m_range->automaticallyClosed)
    {
        text = "AUTO";
        color = {100, 215, 255};
    }
    else if (
        m_checked &&
        m_range->attemptsToComplete <= 0)
    {
        text = "MANUAL";
        color = {120, 255, 105};
    }
    else if (m_isCurrent && !m_checked)
    {
        text = "CURRENT";
        color = {255, 220, 90};
    }

    m_statusLabel->setVisible(!text.empty());

    if (text.empty())
        return;

    m_statusLabel->setString(text.c_str());
    m_statusLabel->setColor(color);
}

void StageRangeCell::updateMetaContent()
{
    if (!m_table)
        return;

    m_table->setScale(1);
    m_table->setVisible(m_isExpanded);
}

void StageRangeCell::onToggle(CCObject *)
{
    if (
        m_disabled ||
        !m_level ||
        m_id.empty())
    {
        if (m_checkbox)
            m_checkbox->toggle(m_checked);

        return;
    }

    auto *profile =
        GlobalStore::get()->getProfileByLevel(
            m_level);

    if (!profile)
    {
        if (m_checkbox)
            m_checkbox->toggle(m_checked);

        return;
    }

    Range *matchedRange = nullptr;

    for (auto &stage : profile->data.stages)
    {
        for (auto &range : stage.ranges)
        {
            if (range.id != m_id)
                continue;

            matchedRange = &range;

            const bool newChecked =
                !range.checked;

            range.checked = newChecked;

            if (!newChecked && range.consider)
            {
                range.attemptsToComplete = 0;
                range.firstRunFrom = 0;
                range.firstRunTo = 0;
                range.completedAt = 0;

                stage.checked = false;
            }

            if (
                newChecked &&
                range.completionCounter <= 0)
            {
                range.completionCounter = 1;
            }

            stage.checked =
                std::all_of(
                    stage.ranges.begin(),
                    stage.ranges.end(),
                    [](const Range &candidate)
                    {
                        return candidate.checked ||
                               !candidate.consider;
                    });

            break;
        }

        if (matchedRange)
            break;
    }

    if (!matchedRange)
    {
        if (m_checkbox)
            m_checkbox->toggle(m_checked);

        return;
    }

    m_range = matchedRange;
    m_checked = matchedRange->checked;

    m_checkbox->toggle(m_checked);

    updateTextColors();
    updateStatusBadge();
    updateTexture();
    updateMetaTable();
    updateLayoutWrapper(true);

    GlobalStore::get()->updateProfile(*profile);
    StageRangesChangedEvent().send();
}

void StageRangeCell::onExpand(CCObject *)
{
    setExpanded(!m_isExpanded, true);
}

void StageRangeCell::setExpanded(
    bool expanded,
    bool triggerCallback)
{
    m_isExpanded = expanded;

    updateMetaContent();
    updateExpandButton();
    updateLayoutWrapper();

    if (
        triggerCallback &&
        onExpandChanged)
    {
        onExpandChanged(
            this,
            m_isExpanded);
    }
}

void StageRangeCell::setDisabled(bool disabled)
{
    m_disabled = disabled;

    if (m_checkbox)
    {
        m_checkbox->setColor(
            m_disabled
                ? ccColor3B{100, 100, 100}
                : ccColor3B{255, 255, 255});
    }

    updateTextColors();
    updateStatusBadge();
    updateTexture();
}
