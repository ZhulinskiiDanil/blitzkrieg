#include "StageRangeCell.hpp"

StageRangeCell *StageRangeCell::create(int from, int to, bool checked, GJGameLevel *level, const CCSize &cellSize)
{
  StageRangeCell *ret = new StageRangeCell();
  if (ret && ret->init(from, to, checked, level, cellSize))
  {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

bool StageRangeCell::init(int from, int to, bool checked, GJGameLevel *level, const CCSize &cellSize)
{
  if (!CCLayer::init())
    return false;

  this->setContentSize(cellSize);
  m_checked = checked;
  m_level = level;
  m_from = from;
  m_to = to;

  // --- Background ---
  m_background = CCScale9Sprite::create("square02b_small.png");
  m_background->setContentSize(cellSize);
  m_background->setPosition({cellSize.width / 2, cellSize.height / 2});
  m_background->setColor({0, 0, 0});
  m_background->setOpacity(255 * 0.3f);
  this->addChild(m_background);

  // --- Range Label ---
  m_rangeLabel = RangeLabel::create(from, to, m_checked);
  m_rangeLabel->setPosition({25.f, cellSize.height / 2});
  this->addChild(m_rangeLabel);

  // --- Checkbox ---
  auto toggleOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

  m_checkbox = CCMenuItemToggler::create(toggleOn, toggleOff, this, menu_selector(StageRangeCell::onToggle));
  m_checkbox->setAnchorPoint({0.5f, 0.5f});
  m_checkbox->toggle(!m_checked);

  auto menu = CCMenu::createWithItem(m_checkbox);
  menu->setScale(0.5f);
  menu->setAnchorPoint({0.5f, 0.f});
  menu->setContentSize({32.f, 32.f});
  menu->setPosition({5.f, cellSize.height / 2});
  this->addChild(menu);

  return true;
}

void StageRangeCell::onToggle(CCObject *sender)
{
  m_checked = !m_checked;
  m_checkbox->toggle(m_checked);
  m_rangeLabel->setEnabled(m_checked);

  if (!m_level)
    return;

  Profile profile = getProfileByLevel(m_level);
  Stage *currentStage = getFirstUncheckedStage(profile);

  if (currentStage)
  {
    for (auto &range : currentStage->ranges)
    {
      if (range.from == m_from && range.to == m_to)
      {
        range.checked = !range.checked;
        break;
      }
    }

    bool everyRangeChecked = std::all_of(currentStage->ranges.begin(), currentStage->ranges.end(),
                                         [](auto &r)
                                         { return r.checked; });

    if (everyRangeChecked)
      currentStage->checked = true;

    saveProfile(profile);
  }
}

void StageRangeCell::setChecked(bool checked)
{
  m_checked = checked;

  if (m_checkbox)
    m_checkbox->toggle(!m_checked);

  if (m_rangeLabel)
    m_rangeLabel->setEnabled(m_checked);

  StagesChangedEvent().post();
}
