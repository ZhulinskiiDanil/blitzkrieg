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
  Profile profile = GlobalStore::get()->getProfileByLevel(m_level);

  if (!profile.id.empty())
  {
    const auto currentRange = GlobalStore::get()->getCurrentRange(profile.id);

    if (!currentRange.id.empty())
      m_isCurrent = currentRange.id == m_range->id;
  }

  updateBackgroundTexture();

  // ! --- Range Label --- !
  const float diff = std::abs(m_range->from - m_range->to);

  m_rangeLabel = Label::create(
      fmt::format(
          "{:.0f}<small>.{:02.0f}%</small> - {:.0f}<small>.{:02.0f}%</small>",
          std::floor(m_range->from),
          std::round((m_range->from - std::floor(m_range->from)) * 100),
          std::floor(m_range->to),
          std::round((m_range->to - std::floor(m_range->to)) * 100)));
  m_rangeLabel->setPosition({25.f, cellSize.height / 2});
  m_rangeLabel->setAnchorPoint({0.f, .5f});
  m_rangeLabel->setZOrder(1);
  this->addChild(m_rangeLabel);
  updateTextColors();

  // ! --- Checkbox --- !
  auto toggleOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

  m_checkbox = CCMenuItemToggler::create(toggleOn, toggleOff, this, menu_selector(StageRangeCell::onToggle));
  m_checkbox->setAnchorPoint({0.5f, 0.5f});
  m_checkbox->setCascadeColorEnabled(true);
  m_checkbox->setZOrder(1);
  m_checkbox->toggle(!m_checked);

  // ! --- Menu --- !
  auto menu = CCMenu::createWithItem(m_checkbox);
  menu->setScale(0.5f);
  menu->setAnchorPoint({0.5f, 0.f});
  menu->setContentSize({32.f, 32.f});
  menu->setPosition({5.f, cellSize.height / 2});
  menu->setZOrder(1);
  this->addChild(menu);

  return true;
}

void StageRangeCell::updateBackgroundTexture()
{
  const auto cellSize = this->getContentSize();
  const auto bg_spr = m_disabled
                          ? "range-disabled-bg.png"_spr
                      : m_checked   ? "range-completed-bg.png"_spr
                      : m_isCurrent ? "range-current-bg.png"_spr
                                    : "range-default-bg.png"_spr;

  m_background = CCScale9Sprite::create(bg_spr);
  m_background->setContentSize(cellSize);
  m_background->setPosition({cellSize.width / 2, cellSize.height / 2});
  m_background->setZOrder(0);

  this->addChild(m_background);
}

void StageRangeCell::onToggle(CCObject *sender)
{
  if (!m_disabled)
    m_checked = !m_checked;

  m_checkbox->toggle(m_checked);

  updateTextColors();
  updateBackgroundTexture();

  if (!m_level || m_disabled)
    return;

  Profile profile = GlobalStore::get()->getProfileByLevel(m_level);
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

    GlobalStore::get()->updateProfile(profile);
  }
}

void StageRangeCell::updateTextColors()
{
  if (m_rangeLabel)
  {
    if (m_checked)
      m_rangeLabel->setVariant(Label::Variant::Green);
    else if (m_disabled)
      m_rangeLabel->setVariant(Label::Variant::Red);
    else
      m_rangeLabel->setVariant(Label::Variant::Orange);
  }
}

void StageRangeCell::setDisabled(bool disabled)
{
  m_disabled = disabled;

  if (m_disabled)
    m_checkbox->setColor({100, 100, 100});
  else
    m_checkbox->setColor({255, 255, 255});

  updateTextColors();
  updateBackgroundTexture();
}