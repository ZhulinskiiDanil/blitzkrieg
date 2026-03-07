#include "ToggablePercentagesList.hpp"

ToggablePercentagesList* ToggablePercentagesList::create(CCSize size, std::vector<float> startposes) {
    auto *ret = new ToggablePercentagesList();
    if (ret && ret->init(size, startposes))
    {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ToggablePercentagesList::init(CCSize size, std::vector<float> startposes) {
    if (!CCLayer::init()) return false;
    this->setContentSize(size);
    this->setAnchorPoint({0.5f, 1.0f});
    this->ignoreAnchorPointForPosition(false);

    auto scrollBorder = ListBorders::create();
    scrollBorder->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f);
    scrollBorder->updateLayout();
    scrollBorder->setContentSize(size);
    scrollBorder->setAnchorPoint({0.5f, 0.5f});
    scrollBorder->setPosition(size / 2.0f);
    this->addChild(scrollBorder);

    for (auto child : CCArrayExt<CCNodeRGBA *>(scrollBorder->getChildren())) {
        child->setColor(ccc3(15, 15, 15));
    }

    auto scrollBG = CCScale9Sprite::create("square02b_001.png");
	scrollBG->setColor({ 0, 0, 0 });
    scrollBG->setOpacity(96);
	scrollBG->setContentSize(scrollBorder->getContentSize() + ccp(0, 5));
	scrollBG->setAnchorPoint({0.5f, 0.5f});
    scrollBG->setPosition(scrollBorder->getPosition());
	// scrollBG->setPosition({scrollBorder->getPositionX(), scrollBorder->getPositionY() - scrollBorder->getContentHeight() / 2.0f});
	scrollBG->ignoreAnchorPointForPosition(false);
    scrollBorder->setZOrder(scrollBG->getZOrder() + 1);
    this->addChild(scrollBG);

    auto startposEditScroll = ScrollLayer::create(scrollBorder->getContentSize() - ccp(10, 10));
    startposEditScroll->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout());
	startposEditScroll->setAnchorPoint({0.5f, 0.5f});
	startposEditScroll->setPosition(scrollBorder->getPosition());
	// startposEditScroll->setPosition({scrollBorder->getPositionX(), scrollBorder->getPositionY() - scrollBorder->getContentHeight() / 2.0f});
	startposEditScroll->ignoreAnchorPointForPosition(false);
    startposEditScroll->setZOrder(scrollBG->getZOrder() + 1);
  
    for (int i = 0; i < (startposes.size() + 2 - 1) / 2; i++) {
        auto row = CCMenu::create();
        row->setContentSize({startposEditScroll->getContentWidth(), 24.0f});
        row->setLayout(RowLayout::create()
        ->setAxisAlignment(AxisAlignment::Between)
        ->setGap(2.5f));
        
        for (int j = i * 2; j < std::min(static_cast<int>(startposes.size()), 2 * (i + 1)); j++) {
        auto cell = CCMenu::create();
        cell->setContentSize({startposEditScroll->getContentWidth() / 2.0f, 24.0f});
        if (static_cast<int>(startposes.size()) < 2 * (i + 1)) cell->setContentSize({startposEditScroll->getContentWidth(), 24.0f});

        auto EnabledCellBG = CCScale9Sprite::create("range-completed-bg.png"_spr);
        EnabledCellBG->setPosition(cell->getContentSize() / 2.0f);
        EnabledCellBG->setContentSize(cell->getContentSize() - ccp(1, 1));
        EnabledCellBG->setID("enabled-cell-bg");
        cell->addChild(EnabledCellBG);

        auto DisabledCellBG = CCScale9Sprite::create("range-disabled-bg.png"_spr);
        DisabledCellBG->setPosition(cell->getContentSize() / 2.0f);
        DisabledCellBG->setContentSize(cell->getContentSize() - ccp(1, 1));
        DisabledCellBG->setVisible(false);
        DisabledCellBG->setID("disabled-cell-bg");
        cell->addChild(DisabledCellBG);

        auto checkbox = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(ToggablePercentagesList::onToggleStartpos), 1.0f);
        checkbox->setScale(0.4f);
        checkbox->setPosition({checkbox->getContentWidth() * checkbox->getScale() / 2.0f + 6.0f, cell->getContentHeight() / 2.0f});
        checkbox->toggle(true);
        checkbox->setTag(j);
        checkbox->setUserObject(cell);
        cell->addChild(checkbox);

        auto percentLabel = Label::create(fmt::format("{:.2f}%", startposes[j]).c_str(), "gjFont17.fnt");
        percentLabel->setVariant(Label::Variant::Green);
        percentLabel->setPosition({checkbox->getPositionX() + checkbox->getContentWidth() * checkbox->getScale() / 2.0f + 4.0f, cell->getContentHeight() / 2.0f});
        percentLabel->setAnchorPoint({0.0f, 0.5f});
        percentLabel->setScale(0.3f);
        percentLabel->setID("percent-label");
        cell->addChild(percentLabel);

        auto runFromLabel = Label::create(
            j == startposes.size()-1 ? fmt::format("<small>{:.2f}% - 100.00%</small>", startposes[j]).c_str() : 
            fmt::format("<small>{:.2f}% - {:.2f}%</small>", startposes[j], startposes[j+1]).c_str(), "gjFont17.fnt");
        runFromLabel->setScale(0.25f);
        runFromLabel->setAnchorPoint({1.0f, 0.0f});
        runFromLabel->setPosition({cell->getContentWidth() - 6.0f, 4.0f});
        cell->addChild(runFromLabel);

        auto runToLabel = Label::create(j == 0 ? fmt::format("<small>0.00% - {:.2f}%</small>", startposes[0]).c_str() : fmt::format("<small>{:.2f}% - {:.2f}%</small>", startposes[j-1], startposes[j]).c_str(), "gjFont17.fnt");
        runToLabel->setScale(0.25f);
        runToLabel->setAnchorPoint({1.0f, 1.0f});
        runToLabel->setPosition({cell->getContentWidth() - 6.0f, cell->getContentHeight() - 4.0f});
        cell->addChild(runToLabel);

        row->addChild(cell);
        row->updateLayout();
        }

        startposEditScroll->m_contentLayer->addChild(row);
    }
    startposEditScroll->m_contentLayer->updateLayout();
    startposEditScroll->moveToTop();
    startposEditScroll->m_peekLimitTop = 24.0f;
    startposEditScroll->m_peekLimitBottom = 24.0f;
    this->addChild(startposEditScroll);

    return true;
}

void ToggablePercentagesList::onToggleStartpos(CCObject *sender)
{
    auto checkbox = static_cast<CCMenuItemToggler*>(sender);
    auto cell = static_cast<CCMenu*>(checkbox->getUserObject());
    auto percentLabel = static_cast<Label*>(cell->getChildByID("percent-label"));
    auto EnabledCellBG = cell->getChildByID("enabled-cell-bg");
    auto DisabledCellBG = cell->getChildByID("disabled-cell-bg");

    if (!checkbox->isToggled()) {
        EnabledCellBG->setVisible(true);
        DisabledCellBG->setVisible(false);
        percentLabel->setVariant(Label::Variant::Green);
    }
    else {
        EnabledCellBG->setVisible(false);
        DisabledCellBG->setVisible(true);
        percentLabel->setVariant(Label::Variant::Red);
    }
}