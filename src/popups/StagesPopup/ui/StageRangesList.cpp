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

  float padding = 3.f;

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(contentSize);
  m_scroll->setContentSize({contentSize.width - padding * 2, contentSize.height - padding * 2});
  m_scroll->setPosition({padding, padding});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(5.f)
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
