#include "StageRangeCell.hpp"

StageRangeCell *StageRangeCell::create(Range *range, GJGameLevel *level, const CCSize &cellSize)
{
  StageRangeCell *ret = new StageRangeCell();
  if (ret && ret->init(range, level, cellSize))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StageRangeCell::init(Range *range, GJGameLevel *level, const CCSize &cellSize)
{
  if (!CCLayer::init())
    return false;

  this->setContentSize(cellSize);
  m_range = range;
  m_checked = m_range->checked;
  m_level = level;

  // --- Background ---
  m_background = CCScale9Sprite::create("square02b_small.png");
  m_background->setContentSize(cellSize);
  m_background->setPosition({cellSize.width / 2, cellSize.height / 2});
  m_background->setColor({17, 16, 16});
  m_background->setOpacity(255 * 0.3f);
  this->addChild(m_background);

  // --- Range Label ---
  m_rangeLabel = RangeLabel::create(m_range);
  m_rangeLabel->setPosition({25.f, cellSize.height / 2});
  this->addChild(m_rangeLabel);

  // --- Checkbox ---
  auto toggleOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

  m_checkbox = CCMenuItemToggler::create(toggleOn, toggleOff, this, menu_selector(StageRangeCell::onToggle));
  m_checkbox->setAnchorPoint({0.5f, 0.5f});
  m_checkbox->setCascadeColorEnabled(true);
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
  if (!m_disabled)
    m_checked = !m_checked;

  m_checkbox->toggle(m_checked);
  m_rangeLabel->setEnabled(m_checked);

  if (!m_level || m_disabled)
    return;

  Profile profile = getProfileByLevel(m_level);
  Stage *currentStage = getFirstUncheckedStage(profile);

  if (currentStage)
  {
    for (auto &range : currentStage->ranges)
    {
      if (range.from == m_range->from && range.to == m_range->to)
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

void StageRangeCell::setDisabled(bool disabled)
{
  m_disabled = disabled;

  if (disabled)
    m_checkbox->setColor({100, 100, 100});
  else
    m_checkbox->setColor({255, 255, 255});
}