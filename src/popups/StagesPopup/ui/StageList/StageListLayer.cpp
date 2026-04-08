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

  float padding = 5.f;
  this->setContentSize(m_contentSize);

  m_listenerStageRangesChanged = StageRangesChangedEvent().listen(
      [this]()
      {
        m_profile = GlobalStore::get()->getProfileByLevel(m_level);
        m_uncheckedStage = getFirstUncheckedStage(*m_profile);

        return ListenerResult::Propagate;
      });

  if (m_stage && m_profile)
  {
    m_stages = &m_profile->data.stages;
    m_currentIndex = m_stage->stage - 1;
    m_uncheckedStage = getFirstUncheckedStage(*m_profile);
  }

  if (!m_stages || m_stages->size() <= 0)
    return true;

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(contentSize);
  m_scroll->setContentSize({contentSize.width - padding * 2, contentSize.height - padding * 2});
  m_scroll->setPosition({padding, padding});
  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));

  this->addChild(m_scroll);

  // ! --- Scroll Content --- !
  m_content = CCLayer::create();
  m_content->setLayout(
      ColumnLayout::create()
          ->setGap(5)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight())
          ->ignoreInvisibleChildren(false));
  m_scroll->m_contentLayer->addChild(m_content);

  // ! --- BG --- !
  RectNode *bg = RectNode::create(contentSize, ccc4FFromccc4B({30, 30, 30, 255}), 8);
  bg->ignoreAnchorPointForPosition(false);
  bg->setAnchorPoint({0.5f, 0.5f});
  bg->setPosition(contentSize / 2);
  bg->setZOrder(-1);
  this->addChild(bg);

  // ! --- Borders --- !
  auto borders = ListBorders::create();
  borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f); // 2.1f
  borders->updateLayout();
  borders->setContentSize({contentSize.width, contentSize.height - 3});
  borders->setPosition({contentSize.width / 2, contentSize.height / 2 - .5f});
  borders->setAnchorPoint({0.5f, 0.5f});

  // ! Set borders color to dark gray
  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
    child->setColor(ccc3(50, 50, 50));

  this->addChild(borders);

  // ! --- Lock --- !
  m_lockSpr = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
  m_lockSpr->setPosition(m_contentSize / 2);
  m_lockSpr->setScale(4.f);
  m_lockSpr->setOpacity(255 * .25f);
  m_lockSpr->setVisible(false);

  this->addChild(m_lockSpr);

  m_listener = StagesChangedEvent().listen(
      [this]()
      {
        reload();
        return ListenerResult::Propagate;
      });
  m_listenerUpdateScrollLayout = UpdateScrollLayoutEvent().listen(
      [this]()
      {
        if (m_content)
        {
          const auto arr = m_content->getChildren();
          CCObject *child;

          for (auto child : CCArrayExt(arr))
          {
            if (auto obj = typeinfo_cast<CCLayer *>(child))
              obj->updateLayout();
          };

          m_content->updateLayout();
          m_scroll->m_contentLayer->updateLayout();
        }

        return ListenerResult::Propagate;
      });

  reload();

  this->addEventListener(
      KeybindSettingPressedEventV3(Mod::get(), "prev-stage-keybind"),
      [this](Keybind const &keybind, bool down, bool repeat, double timestamp)
      {
        if (down && !repeat)
          onPrevStage();
      });

  this->addEventListener(
      KeybindSettingPressedEventV3(Mod::get(), "next-stage-keybind"),
      [this](Keybind const &keybind, bool down, bool repeat, double timestamp)
      {
        if (down && !repeat)
          onNextStage();
      });

  return true;
}

void StageListLayer::reload()
{
  if (!m_stage || !m_content)
    return;

  drawArrows();

  bool isDisabled = m_uncheckedStage && m_stage->stage > m_uncheckedStage->stage;
  m_content->removeAllChildrenWithCleanup(true);
  m_lockSpr->setVisible(isDisabled);

  const float gap = 5.f;
  const float cellHeight = 30.f;
  const float totalWidth = m_scroll->getContentWidth();
  const float cellWidth = (totalWidth - gap) / 2.f;

  std::vector<Range *> visibleRanges;

  for (auto &r : m_stage->ranges)
    if (r.consider)
    {
      if (m_hideCompletedRuns && (r.checked && !m_stage->checked))
        continue;

      visibleRanges.push_back(&r);
    }

  size_t total = visibleRanges.size();
  std::sort(visibleRanges.begin(), visibleRanges.end(),
            [this](const Range *a, const Range *b)
            {
              if (m_sortBy == StageListSortBy::ASC)
                return a->from < b->from;
              else
                return a->from > b->from;
            });

  for (size_t i = 0; i < total;)
  {
    size_t remaining = total - i;
    size_t cellsInRow = std::min<size_t>(2, remaining);

    auto row = CCLayer::create();
    row->setLayout(
        RowLayout::create()
            ->setGap(5)
            ->setAutoScale(false)
            ->setCrossAxisLineAlignment(AxisAlignment::End));
    row->setContentSize({totalWidth, cellHeight});

    for (size_t j = 0; j < cellsInRow; ++j, ++i)
    {
      auto &range = *visibleRanges[i];
      CCSize cellSize = (cellsInRow == 1)
                            ? CCSize(totalWidth, cellHeight)
                            : CCSize(cellWidth, cellHeight);

      auto cell = StageRangeCell::create(&range, m_level, cellSize);
      cell->ignoreAnchorPointForPosition(true);
      cell->setDisabled(isDisabled);

      cell->onExpandChanged = [this, row](StageRangeCell *target, bool expanded)
      {
        for (auto &anotherCell : CCArrayExt<StageRangeCell *>(row->getChildren()))
          if (anotherCell != target)
            anotherCell->setExpanded(expanded);

        row->updateLayout();
      };

      row->addChild(cell);
      row->updateLayout();
    }

    m_content->addChild(row);
  }

  m_content->updateLayout();
  m_scroll->m_contentLayer->updateLayout();
  scrollToTop();
}

void StageListLayer::setSortBy(StageListSortBy sortBy)
{
  m_sortBy = sortBy;
}

void StageListLayer::setRunsVisabilityForCompleted(bool visible)
{
  m_hideCompletedRuns = visible;
}

void StageListLayer::drawArrows()
{
  const auto stagesMetaInfo = getMetaInfoFromStages(*m_stages);

  if (!m_stages || !m_stage || stagesMetaInfo.consideredStages->empty() || stagesMetaInfo.consideredStages->size() < 2)
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
      menu_selector(StageListLayer::onPrevStageBtn));
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
      menu_selector(StageListLayer::onNextStageBtn));
  m_buttonMenuRight->addChild(m_buttonRight);
  m_buttonMenuRight->updateLayout();

  m_buttonMenuRight->setVisible(m_stage->stage < stagesMetaInfo.total);
  this->addChild(m_buttonMenuRight);
}

void StageListLayer::scrollToTop()
{
  if (m_scroll)
    m_scroll->scrollToTop();
}

void StageListLayer::onPrevStageBtn(CCObject *sender)
{
  onPrevStage();
}

void StageListLayer::onNextStageBtn(CCObject *sender)
{
  onNextStage();
}

void StageListLayer::onPrevStage()
{
  const auto stagesMetaInfo = getMetaInfoFromStages(*m_stages);

  if (!m_stages || stagesMetaInfo.consideredStages->empty() || stagesMetaInfo.consideredStages->size() < 2)
    return;

  if (m_currentIndex > 0)
    --m_currentIndex;

  m_stage = &stagesMetaInfo.consideredStages->at(m_currentIndex);
  StageSwitchedEvent().send(stagesMetaInfo.consideredStages->size(), m_stage);
  reload();
}

void StageListLayer::onNextStage()
{
  const auto stagesMetaInfo = getMetaInfoFromStages(*m_stages);

  if (!m_stages || stagesMetaInfo.consideredStages->empty() || stagesMetaInfo.consideredStages->size() < 2)
    return;

  if (m_currentIndex + 1 < static_cast<int>(stagesMetaInfo.total))
    ++m_currentIndex;

  m_stage = &stagesMetaInfo.consideredStages->at(m_currentIndex);
  StageSwitchedEvent().send(stagesMetaInfo.consideredStages->size(), m_stage);
  reload();
}
