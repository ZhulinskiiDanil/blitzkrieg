#include "ToggablePercentagesList.hpp"

ToggablePercentagesList *ToggablePercentagesList::create(CCSize size, std::vector<float> startposes, std::vector<float> disabledStartposes)
{
	auto *ret = new ToggablePercentagesList();
	if (ret && ret->init(size, startposes, disabledStartposes))
	{
		ret->autorelease();
		return ret;
	}

	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool ToggablePercentagesList::init(CCSize size, std::vector<float> startposes, std::vector<float> disabledStartposes)
{
	if (!CCLayer::init())
		return false;
	this->setContentSize(size);
	this->setAnchorPoint({0.5f, 1.0f});
	this->ignoreAnchorPointForPosition(false);

	m_startposes = startposes;
	m_enabledStartposes = startposes;
	std::sort(m_startposes.begin(), m_startposes.end());

	for (float disabledStartpos : disabledStartposes)
		m_enabledStartposes.erase(std::remove(m_enabledStartposes.begin(), m_enabledStartposes.end(), disabledStartpos), m_enabledStartposes.end());

	// ! --- Sroll Border --- !
	auto scrollBorder = ListBorders::create();
	scrollBorder->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f);
	scrollBorder->updateLayout();
	scrollBorder->setContentSize(size);
	scrollBorder->setAnchorPoint({0.5f, 0.5f});
	scrollBorder->setPosition(size / 2.0f);
	this->addChild(scrollBorder);

	for (auto child : CCArrayExt<CCNodeRGBA *>(scrollBorder->getChildren()))
	{
		child->setColor(ccc3(15, 15, 15));
	}

	// ! --- Sroll Background --- !
	auto scrollBG = CCScale9Sprite::create("square02b_001.png");
	scrollBG->setColor({0, 0, 0});
	scrollBG->setOpacity(96);
	scrollBG->setContentSize(scrollBorder->getContentSize() + ccp(0, 5));
	scrollBG->setAnchorPoint({0.5f, 0.5f});
	scrollBG->setPosition(scrollBorder->getPosition());
	scrollBG->ignoreAnchorPointForPosition(false);
	scrollBorder->setZOrder(scrollBG->getZOrder() + 1);
	this->addChild(scrollBG);

	// ! --- Sroll Layer --- !
	m_scroll = ScrollLayer::create(scrollBorder->getContentSize() - ccp(10, 10));
	m_scroll->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout());
	m_scroll->setAnchorPoint({0.5f, 0.5f});
	m_scroll->setPosition(scrollBorder->getPosition());
	m_scroll->ignoreAnchorPointForPosition(false);
	m_scroll->setZOrder(scrollBG->getZOrder() + 1);
	m_scroll->m_peekLimitTop = 24.0f;
	m_scroll->m_peekLimitBottom = 24.0f;
	this->addChild(m_scroll);

	reload();

	return true;
}

void ToggablePercentagesList::reload()
{
	m_scroll->m_contentLayer->removeAllChildrenWithCleanup(true);

	for (int i = 0; i < (m_startposes.size() + 2 - 1) / 2; i++)
	{
		auto row = CCLayer::create();
		row->setContentSize({m_scroll->getContentWidth(), 24.0f});
		row->setLayout(RowLayout::create()
											 ->setAxisAlignment(AxisAlignment::Between)
											 ->setGap(2.5f));

		for (int j = i * 2; j < std::min(static_cast<int>(m_startposes.size()), 2 * (i + 1)); j++)
		{
			auto isStartposEnabled = std::find(m_enabledStartposes.begin(), m_enabledStartposes.end(), m_startposes[j]) != m_enabledStartposes.end();
			auto cell = CCLayer::create();
			cell->setContentSize({m_scroll->getContentWidth() / 2.0f, 24.0f});
			if (static_cast<int>(m_startposes.size()) < 2 * (i + 1))
				cell->setContentSize({m_scroll->getContentWidth(), 24.0f});

			// ! --- Cell Background --- !
			auto EnabledCellBG = CCScale9Sprite::create("range-completed-bg.png"_spr);
			EnabledCellBG->setPosition(cell->getContentSize() / 2.0f);
			EnabledCellBG->setContentSize(cell->getContentSize() - ccp(1, 1));
			EnabledCellBG->setVisible(isStartposEnabled);
			EnabledCellBG->setID("enabled-cell-bg");
			cell->addChild(EnabledCellBG);

			// ! --- Cell Background (Disabled) --- !
			auto DisabledCellBG = CCScale9Sprite::create("range-disabled-bg.png"_spr);
			DisabledCellBG->setPosition(cell->getContentSize() / 2.0f);
			DisabledCellBG->setContentSize(cell->getContentSize() - ccp(1, 1));
			DisabledCellBG->setVisible(!isStartposEnabled);
			DisabledCellBG->setID("disabled-cell-bg");
			cell->addChild(DisabledCellBG);

			// ! --- Checkbox Menu --- !
			auto checkboxMenu = CCMenu::create();
			checkboxMenu->setContentSize({12, 12});
			checkboxMenu->setAnchorPoint({0, 0.5f});
			checkboxMenu->ignoreAnchorPointForPosition(false);
			checkboxMenu->setPosition({6, cell->getContentHeight() / 2.0f});

			// ! --- Checkbox --- !
			auto checkbox = CCMenuItemToggler::createWithStandardSprites(
					this, menu_selector(ToggablePercentagesList::onToggleStartpos), 1.0f);
			checkbox->setScale(0.4f);
			checkbox->setPosition(checkboxMenu->getContentSize() / 2);
			checkbox->setTag(j);

			if (isStartposEnabled)
				checkbox->toggle(true);

			checkboxMenu->addChild(checkbox);
			cell->addChild(checkboxMenu);

			// ! --- Percentage Label --- !
			auto percentLabel = Label::create(fmt::format("{:.2f}%", m_startposes[j]).c_str(), "gjFont17.fnt");
			percentLabel->setVariant(isStartposEnabled ? Label::Variant::Green : Label::Variant::Red);
			percentLabel->setPosition({checkboxMenu->getPositionX() + checkboxMenu->getContentWidth() + 4.0f, cell->getContentHeight() / 2.0f});
			percentLabel->setAnchorPoint({0.0f, 0.5f});
			percentLabel->setScale(0.3f);
			percentLabel->setID("percent-label");
			cell->addChild(percentLabel);

			// ! --- Checkbox Menu --- !
			auto buttonMenu = CCMenu::create();
			buttonMenu->setAnchorPoint({1, 0.5f});
			buttonMenu->ignoreAnchorPointForPosition(false);
			buttonMenu->setPosition({cell->getContentWidth() - 6, cell->getContentHeight() / 2.0f});
			buttonMenu->setLayout(
					RowLayout::create()
							->setGap(2.f)
							->setAutoScale(false)
							->setAutoGrowAxis(true)
							->setAxisAlignment(AxisAlignment::Center));

			// ! --- Edit button --- !
			auto spr = CCSprite::createWithSpriteFrameName("edit-profile-btn.png"_spr);
			const auto editBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ToggablePercentagesList::onEditStartpos));
			editBtn->setTag(j);
			editBtn->setScale(0.4f);
			editBtn->m_baseScale = 0.4f;

			buttonMenu->addChild(editBtn);
			cell->addChild(buttonMenu);
			buttonMenu->updateLayout();

			// ! --- Range Labels --- !
			auto runFromLabel = Label::create(
					j == m_startposes.size() - 1 ? fmt::format("<small>{:.2f}% - 100.00%</small>", m_startposes[j]).c_str() : fmt::format("<small>{:.2f}% - {:.2f}%</small>", m_startposes[j], m_startposes[j + 1]).c_str(), "gjFont17.fnt");
			runFromLabel->setScale(0.25f);
			runFromLabel->setAnchorPoint({1.0f, 0.0f});
			runFromLabel->setPosition({cell->getContentWidth() - buttonMenu->getContentWidth() - 6 - 2, 4.0f});
			cell->addChild(runFromLabel);

			// ! --- Run To Label --- !
			auto runToLabel = Label::create(j == 0 ? fmt::format("<small>0.00% - {:.2f}%</small>", m_startposes[0]).c_str() : fmt::format("<small>{:.2f}% - {:.2f}%</small>", m_startposes[j - 1], m_startposes[j]).c_str(), "gjFont17.fnt");
			runToLabel->setScale(0.25f);
			runToLabel->setAnchorPoint({1.0f, 1.0f});
			runToLabel->setPosition({cell->getContentWidth() - buttonMenu->getContentWidth() - 6 - 2, cell->getContentHeight() - 4.0f});
			cell->addChild(runToLabel);

			row->addChild(cell);
			row->updateLayout();
		}

		m_scroll->m_contentLayer->addChild(row);
	}

	m_scroll->m_contentLayer->updateLayout();
	m_scroll->moveToTop();
}

void ToggablePercentagesList::setStartposes(std::vector<float> startposes)
{
	m_startposes = startposes;
	m_enabledStartposes = startposes;
	reload();
}

std::vector<float> ToggablePercentagesList::getStartposes()
{
	return m_startposes;
}

std::vector<float> ToggablePercentagesList::getEnabledStartposes()
{
	return m_enabledStartposes;
}

void ToggablePercentagesList::onToggleStartpos(CCObject *sender)
{
	auto checkbox = static_cast<CCMenuItemToggler *>(sender);
	auto cell = static_cast<CCLayer *>(static_cast<CCMenu *>(checkbox->getParent())->getParent());
	auto percentLabel = static_cast<Label *>(cell->getChildByID("percent-label"));
	auto EnabledCellBG = cell->getChildByID("enabled-cell-bg");
	auto DisabledCellBG = cell->getChildByID("disabled-cell-bg");

	if (!checkbox->isToggled())
	{
		EnabledCellBG->setVisible(true);
		DisabledCellBG->setVisible(false);
		percentLabel->setVariant(Label::Variant::Green);

		m_enabledStartposes.push_back(m_startposes[checkbox->getTag()]);
		std::sort(m_enabledStartposes.begin(), m_enabledStartposes.end());
	}
	else
	{
		EnabledCellBG->setVisible(false);
		DisabledCellBG->setVisible(true);
		percentLabel->setVariant(Label::Variant::Red);

		m_enabledStartposes.erase(std::remove(m_enabledStartposes.begin(), m_enabledStartposes.end(), m_startposes[checkbox->getTag()]), m_enabledStartposes.end());
		std::sort(m_enabledStartposes.begin(), m_enabledStartposes.end());
	}
}

void ToggablePercentagesList::onEditStartpos(CCObject *sender)
{
	auto checkbox = static_cast<CCMenuItemToggler *>(sender);
	float percentage = m_startposes[checkbox->getTag()];

	EditStartPosPopup::create(
			percentage,
			[this](float oldPercent, float newPercent)
			{
				m_enabledStartposes.erase(std::remove(m_enabledStartposes.begin(), m_enabledStartposes.end(), oldPercent), m_enabledStartposes.end());

				for (auto &p : m_startposes)
				{
					if (p == oldPercent)
					{
						p = newPercent;
						m_enabledStartposes.push_back(newPercent);
						std::sort(m_enabledStartposes.begin(), m_enabledStartposes.end());
						break;
					}
				}

				m_startposes.erase(std::remove(m_startposes.begin(), m_startposes.end(), oldPercent), m_startposes.end());
				this->reload();
			})
			->show();
}