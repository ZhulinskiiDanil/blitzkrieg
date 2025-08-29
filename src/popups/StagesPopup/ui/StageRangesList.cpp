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

  const float gap = 5.f;
  const float cellHeight = 30.f;
  const float totalWidth = m_scroll->getContentWidth();
  const float cellWidth = (totalWidth - gap) / 2.f;

  size_t total = m_stage->ranges.size();

  for (size_t i = 0; i < total;)
  {
    size_t remaining = total - i;
    size_t cellsInRow = (remaining == 1) ? 1 : 2;

    auto row = CCLayer::create();
    row->setContentSize({totalWidth, cellHeight});

    for (size_t j = 0; j < cellsInRow; ++j, ++i)
    {
      auto &range = m_stage->ranges[i];
      CCSize cellSize = (cellsInRow == 1)
                            ? CCSize(totalWidth, cellHeight)
                            : CCSize(cellWidth, cellHeight);

      auto cell = StageRangeCell::create(&range, m_level, cellSize);

      float x = (cellsInRow == 2)
                    ? (j != 0 ? 0.f : (cellWidth + gap))
                    : 0.f;

      cell->setAnchorPoint({0, 0});
      cell->setPosition(x, 0);

      row->addChild(cell);
    }

    m_scroll->m_contentLayer->addChild(row);
  }

  m_scroll->m_contentLayer->updateLayout();
  scrollToTop();
}

void StageRangesList::scrollToTop()
{
  if (m_scroll)
    m_scroll->scrollToTop();
}
