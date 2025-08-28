#include "StageRangesList.hpp"

StageRangesList *StageRangesList::create(Stage *stage, GJGameLevel *level, const CCSize &contentSize)
{
  StageRangesList *ret = new StageRangesList();
  if (ret && ret->init(stage, level, contentSize))
  {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

bool StageRangesList::init(Stage *stage, GJGameLevel *level, const CCSize &contentSize)
{
  if (!CCLayer::init())
    return false;

  m_contentSize = contentSize;
  m_level = level;
  m_stage = stage;

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(contentSize);
  m_scroll->setContentSize({contentSize.width - 16, contentSize.height - 16});
  m_scroll->setPosition({8, 8});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(5.f)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));

  this->addChild(m_scroll);

  // ! --- Borders --- !
  auto borders = ListBorders::create();
  borders->setContentSize(contentSize);
  borders->setAnchorPoint({0.5f, 0.5f});
  borders->setPosition(contentSize / 2);
  this->addChild(borders);

  reload();

  m_listener = EventListener<EventFilter<StagesChangedEvent>>(
      [this](StagesChangedEvent *)
      {
        reload();
        return ListenerResult::Propagate;
      });

  return true;
}

void StageRangesList::reload()
{
  if (!m_stage)
    return;

  m_scroll->m_contentLayer->removeAllChildrenWithCleanup(true);

  for (size_t i = 0; i < m_stage->ranges.size(); ++i)
  {
    auto &range = m_stage->ranges[i];
    auto cellSize = CCSize(m_scroll->getContentWidth(), 25.f);

    auto cell = StageRangeCell::create(range.from, range.to, range.checked, m_level, cellSize);
    m_scroll->m_contentLayer->addChild(cell);
  }

  m_scroll->m_contentLayer->updateLayout();
  scrollToTop();
}

void StageRangesList::scrollToTop()
{
  if (m_scroll)
    m_scroll->scrollToTop();
}
