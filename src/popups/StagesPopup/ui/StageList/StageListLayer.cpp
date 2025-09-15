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
  m_profile = getProfileByLevel(m_level);
  m_uncheckedStage = getFirstUncheckedStage(m_profile);

  m_stage = stage;
  m_stages = &m_profile.data.stages;
  m_currentIndex = m_stage->stage - 1;

  this->setContentSize(m_contentSize);

  float margin = 3.f;

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(contentSize);
  m_scroll->setContentSize({contentSize.width - margin * 2, contentSize.height - margin * 2});
  m_scroll->setPosition({margin, margin});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(5.f)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAxisReverse(true)
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

  m_listener = EventListener<EventFilter<StagesChangedEvent>>(
      [this](StagesChangedEvent *)
      {
        reload();
        return ListenerResult::Propagate;
      });

  reload();
  drawArrows();

  return true;
}

void StageListLayer::reload()
{
  if (!m_stage)
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
            ->setAutoScale(false));
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
  m_buttonMenuLeft->setPosition({this->getPositionX() - 20.f,
                                 this->getPositionY() + this->getContentHeight() / 2});
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

  m_buttonMenuLeft->setVisible(m_stage->stage > 0);
  this->addChild(m_buttonMenuLeft);

  // ! --- Right Arrow Button --- !
  m_buttonMenuRight = CCMenu::create();
  m_buttonMenuRight->setAnchorPoint({0.f, .5f});
  m_buttonMenuRight->setPosition({this->getPositionX() + this->getContentWidth() + 20.f,
                                  this->getPositionY() + this->getContentHeight() / 2});
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
  StageChangedEvent(m_stage->stage, m_stages->size()).post();
  reload();
}

void StageListLayer::onNextStage(CCObject *sender)
{
  if (!m_stages || m_stages->empty() || m_stages->size() < 2)
    return;

  if (m_currentIndex + 1 < static_cast<int>(m_stages->size()))
    ++m_currentIndex;

  m_stage = &m_stages->at(m_currentIndex);
  StageChangedEvent(m_stage->stage, m_stages->size()).post();
  reload();
}