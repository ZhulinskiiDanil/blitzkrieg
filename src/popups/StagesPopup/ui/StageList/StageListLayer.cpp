#include "StageListLayer.hpp"

StageListLayer *StageListLayer::create(
    Stage *stage,
    GJGameLevel *level,
    const CCSize &contentSize)
{
  auto *ret = new StageListLayer();
  if (ret && ret->init(stage, level, contentSize))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StageListLayer::init(
    Stage *stage,
    GJGameLevel *level,
    const CCSize &contentSize)
{
  if (!CCLayer::init())
    return false;

  m_contentSize = contentSize;
  m_level = level;
  m_profile = GlobalStore::get()->getProfileByLevel(m_level);
  m_stage = stage;

  if (m_stage && !m_profile.id.empty())
  {
    m_stages = &m_profile.data.stages;
    m_currentIndex = m_stage->stage - 1;
    m_uncheckedStage = getFirstUncheckedStage(m_profile);

    if (!m_uncheckedStage && !m_profile.data.stages.empty())
      m_uncheckedStage = &m_profile.data.stages.back();
  }

  this->setContentSize(m_contentSize);

  float padding = 5.f;

  if (!m_stages || m_stages->size() <= 0)
  {
    auto bigFontLabel = CCLabelBMFont::create("First attach the profile!", "bigFont.fnt");
    bigFontLabel->setScale(.75f);
    bigFontLabel->setOpacity(255 * .6f);
    bigFontLabel->setPosition(contentSize / 2);

    this->addChild(bigFontLabel);
    return true;
  }

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(contentSize);
  m_scroll->setContentSize({contentSize.width - padding * 2, contentSize.height - padding * 2});
  m_scroll->setPosition({padding, padding});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(2.5f)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));

  this->addChild(m_scroll);

  // ! --- Borders --- !
  auto borders = ListBorders::create();
  borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f); // 2.1f
  borders->updateLayout();
  borders->setContentSize({contentSize.width, contentSize.height - 3});
  borders->setPosition({contentSize.width / 2, contentSize.height / 2 - .5f});
  borders->setAnchorPoint({0.5f, 0.5f});

  this->addChild(borders);

  // ! --- Lock --- !
  m_lockSpr = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
  m_lockSpr->setPosition(m_contentSize / 2);
  m_lockSpr->setScale(4.f);
  m_lockSpr->setVisible(false);

  this->addChild(m_lockSpr);

  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
  {
    child->setColor(ccc3(15, 15, 15));
  }

  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
  {
    child->setColor(ccc3(15, 15, 15));
  }

  m_listener = StagesChangedEvent().listen(
      [this]()
      {
        reload();
        return ListenerResult::Propagate;
      });
  m_listenerUpdateScrollLayout = UpdateScrollLayoutEvent().listen(
      [this]()
      {
        const auto arr = m_scroll->m_contentLayer->getChildren();
        CCObject *child;

        for (auto child : CCArrayExt(arr))
        {
          if (auto obj = typeinfo_cast<CCLayer *>(child))
            obj->updateLayout();
        };

        m_scroll->m_contentLayer->updateLayout();
        return ListenerResult::Propagate;
      });

  reload();

  return true;
}

void StageListLayer::reload()
{
  drawArrows();

  if (!m_stage || !m_uncheckedStage)
    return;

  bool isDisabled = m_stage->stage > m_uncheckedStage->stage;
  m_scroll->m_contentLayer->removeAllChildrenWithCleanup(true);
  m_lockSpr->setVisible(isDisabled);

  const float gap = 5.f;
  const float cellHeight = 30.f;
  const float totalWidth = m_scroll->getContentWidth();
  const float cellWidth = (totalWidth - gap) / 2.f;

  size_t total = m_stage->ranges.size();

  for (size_t i = 0; i < total;)
  {
    size_t remaining = total - i;
    size_t cellsInRow = std::min<size_t>(2, remaining);

    auto row = CCLayer::create();
    row->setLayout(
        RowLayout::create()
            ->setGap(5.f)
            ->setAutoScale(false)
            ->setCrossAxisLineAlignment(AxisAlignment::End));
    row->setContentSize({totalWidth, cellHeight});

    for (size_t j = 0; j < cellsInRow; ++j, ++i)
    {
      auto &range = m_stage->ranges[i];
      CCSize cellSize = (cellsInRow == 1)
                            ? CCSize(totalWidth, cellHeight)
                            : CCSize(cellWidth, cellHeight);

      auto cell = StageRangeCell::create(&range, m_level, cellSize);
      cell->ignoreAnchorPointForPosition(true);
      cell->setDisabled(isDisabled);

      row->addChild(cell);
      row->updateLayout();
    }

    m_scroll->m_contentLayer->addChild(row);
  }

  m_scroll->m_contentLayer->updateLayout();
  scrollToTop();
}

void StageListLayer::drawArrows()
{
  if (!m_stages || m_stages->empty() || m_stages->size() < 2)
    return;

  if (m_buttonMenuLeft)
    m_buttonMenuLeft->removeFromParentAndCleanup(true);
  if (m_buttonMenuRight)
    m_buttonMenuRight->removeFromParentAndCleanup(true);

  // ! --- Left Arrow Button --- !
  m_buttonMenuLeft = CCMenu::create();
  m_buttonMenuLeft->setAnchorPoint({1.f, .5f});
  m_buttonMenuLeft->setPosition({-15.f,
                                 this->getContentHeight() / 2});
  m_buttonMenuLeft->setLayout(
      RowLayout::create()
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::Center)
          ->setCrossAxisAlignment(AxisAlignment::Center));

  const auto btnLeftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
  m_buttonLeft = CCMenuItemSpriteExtra::create(
      btnLeftSpr,
      this,
      menu_selector(StageListLayer::onPrevStage));
  m_buttonMenuLeft->addChild(m_buttonLeft);
  m_buttonMenuLeft->updateLayout();

  m_buttonMenuLeft->setVisible(m_stage->stage > 1);
  this->addChild(m_buttonMenuLeft);

  // ! --- Right Arrow Button --- !
  m_buttonMenuRight = CCMenu::create();
  m_buttonMenuRight->setAnchorPoint({0.f, .5f});
  m_buttonMenuRight->setPosition({this->getContentWidth() + 15.f,
                                  this->getContentHeight() / 2});
  m_buttonMenuRight->setLayout(
      m_buttonMenuLeft->getLayout());

  const auto btnRightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
  btnRightSpr->setFlipX(true);
  m_buttonRight = CCMenuItemSpriteExtra::create(
      btnRightSpr,
      this,
      menu_selector(StageListLayer::onNextStage));
  m_buttonMenuRight->addChild(m_buttonRight);
  m_buttonMenuRight->updateLayout();

  m_buttonMenuRight->setVisible(m_stage->stage < m_stages->size());
  this->addChild(m_buttonMenuRight);
}

void StageListLayer::scrollToTop()
{
  if (m_scroll)
    m_scroll->scrollToTop();
}

void StageListLayer::onPrevStage(CCObject *sender)
{
  if (!m_stages || m_stages->empty() || m_stages->size() < 2)
    return;

  if (m_currentIndex > 0)
    --m_currentIndex;

  m_stage = &m_stages->at(m_currentIndex);
  StageChangedEvent().send(m_stages->size(), m_stage);
  reload();
}

void StageListLayer::onNextStage(CCObject *sender)
{
  if (!m_stages || m_stages->empty() || m_stages->size() < 2)
    return;

  if (m_currentIndex + 1 < static_cast<int>(m_stages->size()))
    ++m_currentIndex;

  m_stage = &m_stages->at(m_currentIndex);
  StageChangedEvent().send(m_stages->size(), m_stage);
  reload();
}
