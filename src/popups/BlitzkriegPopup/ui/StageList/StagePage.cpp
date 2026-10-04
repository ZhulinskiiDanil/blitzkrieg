#include "StagePage.hpp"

#include <algorithm>

StagePage *StagePage::create(
    GJGameLevel *level,
    const CCSize &size,
    std::unordered_map<std::string, bool> *expandedRanges)
{
  auto *ret = new StagePage();
  if (ret && ret->init(level, size, expandedRanges))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StagePage::init(
    GJGameLevel *level,
    const CCSize &size,
    std::unordered_map<std::string, bool> *expandedRanges)
{
  if (!CCNode::init())
    return false;

  m_size = size;
  m_level = level;
  m_expandedRanges = expandedRanges;

  this->setContentSize(size);

  const float padding = 5.f;

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(size);
  m_scroll->setContentSize({size.width - padding * 2, size.height - padding * 2});
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

  createLockPanel();
  createEmptyState();

  // Cells animate their height and ask the list to follow
  m_listenerUpdateScrollLayout = UpdateScrollLayoutEvent().listen(
      [this]()
      {
        for (auto child : CCArrayExt<CCNode *>(m_content->getChildren()))
        {
          if (auto obj = typeinfo_cast<CCLayer *>(child))
            obj->updateLayout();
        };

        m_content->updateLayout();
        m_scroll->m_contentLayer->updateLayout();

        return ListenerResult::Propagate;
      });

  return true;
}

void StagePage::createLockPanel()
{
  m_lockPanel = CCNode::create();
  m_lockPanel->setAnchorPoint({.5f, .5f});
  m_lockPanel->setPosition(m_size / 2);
  m_lockPanel->setVisible(false);
  this->addChild(m_lockPanel, 2);

  // ! --- Background: outline + opaque fill, cells do not show through --- !
  m_lockPanelBorder = RectNode::create(
      {1.f, 1.f},
      ccc4FFromccc4B({70, 70, 70, 255}),
      LOCK_PANEL_RADIUS);
  m_lockPanel->addChild(m_lockPanelBorder);

  m_lockPanelFill = RectNode::create(
      {1.f, 1.f},
      ccc4FFromccc4B({22, 22, 22, 255}),
      LOCK_PANEL_RADIUS - LOCK_PANEL_OUTLINE);
  m_lockPanelFill->setPosition({LOCK_PANEL_OUTLINE, LOCK_PANEL_OUTLINE});
  m_lockPanel->addChild(m_lockPanelFill);

  // ! --- Icon --- !
  m_lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
  m_lockIcon->setScale(.6f);
  m_lockPanel->addChild(m_lockIcon);

  // ! --- Text --- !
  m_lockLabel = CCLabelBMFont::create("", "bigFont.fnt");
  m_lockPanel->addChild(m_lockLabel);

  // ! --- Button --- !
  auto btnSpr = ButtonSprite::create("Go to current");
  m_lockButton = CCMenuItemSpriteExtra::create(
      btnSpr, this, menu_selector(StagePage::onGoToCurrent));
  m_lockButton->setScale(.5f);
  m_lockButton->m_baseScale = .5f;

  m_lockMenu = CCMenu::createWithItem(m_lockButton);
  m_lockPanel->addChild(m_lockMenu);

  layoutLockPanel();
}

void StagePage::layoutLockPanel()
{
  const float iconHeight = m_lockIcon->getScaledContentHeight();
  const float labelHeight = m_lockLabel->getScaledContentHeight();
  const float buttonHeight = m_lockButton->getScaledContentHeight();

  const CCSize panelSize{
      LOCK_PANEL_WIDTH,
      LOCK_PANEL_PADDING * 2 + iconHeight + labelHeight + buttonHeight + LOCK_PANEL_GAP * 2,
  };

  m_lockPanel->setContentSize(panelSize);
  m_lockPanelBorder->setSize(panelSize);
  m_lockPanelFill->setSize({
      panelSize.width - LOCK_PANEL_OUTLINE * 2,
      panelSize.height - LOCK_PANEL_OUTLINE * 2,
  });

  // Top to bottom: icon, text, button
  const float centerX = panelSize.width / 2;
  float y = panelSize.height - LOCK_PANEL_PADDING;

  m_lockIcon->setPosition({centerX, y - iconHeight / 2});
  y -= iconHeight + LOCK_PANEL_GAP;

  m_lockLabel->setPosition({centerX, y - labelHeight / 2});
  y -= labelHeight + LOCK_PANEL_GAP;

  // The menu holds the button at its origin
  m_lockMenu->setPosition({centerX, y - buttonHeight / 2});
}

void StagePage::createEmptyState()
{
  m_emptyState = CCNode::create();
  m_emptyState->setPosition(m_size / 2);
  m_emptyState->setVisible(false);
  this->addChild(m_emptyState, 1);

  auto title = CCLabelBMFont::create("All runs completed", "bigFont.fnt");
  title->setScale(.5f);
  title->setOpacity(200);
  title->setPosition({0.f, 8.f});
  m_emptyState->addChild(title);

  auto hint = CCLabelBMFont::create(
      "Turn off the filter to show them",
      "bigFont.fnt");
  hint->setScale(.3f);
  hint->setOpacity(120);
  hint->setPosition({0.f, -8.f});
  hint->limitLabelWidth(m_size.width - 20.f, .3f, .15f);
  m_emptyState->addChild(hint);
}

void StagePage::build(
    Stage *stage,
    int stageIndex,
    int currentIndex,
    StagePageOptions const &options,
    bool keepScroll,
    std::string const &focusRangeId)
{
  m_stageIndex = stage ? stageIndex : -1;

  auto *contentLayer = m_scroll->m_contentLayer;
  const float viewHeight = m_scroll->getContentHeight();

  // Content layer is at (viewHeight - contentHeight) when scrolled to the top
  const float distanceFromTop =
      contentLayer->getPositionY() - (viewHeight - contentLayer->getContentHeight());

  m_content->removeAllChildrenWithCleanup(true);

  const bool isLocked = stage && currentIndex >= 0 && stageIndex > currentIndex;

  m_lockPanel->setVisible(isLocked);
  m_emptyState->setVisible(false);

  if (isLocked)
  {
    m_lockLabel->setString(
        fmt::format("Complete stage {} to unlock", currentIndex + 1).c_str());
    m_lockLabel->limitLabelWidth(LOCK_PANEL_WIDTH - LOCK_PANEL_PADDING * 2, .35f, .2f);
    layoutLockPanel();
  }

  if (!stage)
  {
    m_content->updateLayout();
    contentLayer->updateLayout();
    return;
  }

  // ! --- Visible runs --- !
  std::vector<Range *> visibleRanges;
  int hiddenCount = 0;

  for (auto &r : stage->ranges)
  {
    if (!r.consider)
      continue;

    // The requested run is shown even when the filter hides it
    if (options.hideCompletedRuns && (r.checked && !stage->checked) && r.id != focusRangeId)
    {
      hiddenCount++;
      continue;
    }

    visibleRanges.push_back(&r);
  }

  m_emptyState->setVisible(visibleRanges.empty() && !isLocked);

  std::sort(visibleRanges.begin(), visibleRanges.end(),
            [&options](const Range *a, const Range *b)
            {
              if (options.sortBy == StageListSortBy::ASC)
                return a->from < b->from;
              else
                return a->from > b->from;
            });

  // ! --- Current run, the page scrolls to it --- !
  std::string currentRangeId;

  if (auto *profile = GlobalStore::get()->getProfileByLevel(m_level))
    currentRangeId = GlobalStore::get()->getCurrentRange(profile->id).id;

  CCNode *currentRow = nullptr;
  // The requested run wins over the current one
  CCNode *focusRow = nullptr;
  StageRangeCell *focusCell = nullptr;

  // ! --- Rows --- !
  const float gap = 5.f;
  const float cellHeight = 30.f;
  const float totalWidth = m_scroll->getContentWidth();
  const float cellWidth = (totalWidth - gap) / 2.f;

  const bool expandedByDefault =
      Mod::get()->getSettingValue<bool>("expand-progress-by-default");

  auto isRangeExpanded = [this, expandedByDefault](Range const &range)
  {
    if (!m_expandedRanges)
      return expandedByDefault;

    auto it = m_expandedRanges->find(range.id);
    return it != m_expandedRanges->end() ? it->second : expandedByDefault;
  };

  const size_t total = visibleRanges.size();

  for (size_t i = 0; i < total;)
  {
    const size_t cellsInRow = std::min<size_t>(2, total - i);

    // Cells of a row share the height, so the row is expanded if any of them is
    bool rowExpanded = false;

    for (size_t j = 0; j < cellsInRow; ++j)
      rowExpanded = rowExpanded || isRangeExpanded(*visibleRanges[i + j]);

    auto row = CCLayer::create();
    row->setLayout(
        RowLayout::create()
            ->setGap(gap)
            ->setAutoScale(false)
            ->setCrossAxisLineAlignment(AxisAlignment::End));
    row->setContentSize({totalWidth, cellHeight});

    for (size_t j = 0; j < cellsInRow; ++j, ++i)
    {
      auto &range = *visibleRanges[i];
      CCSize cellSize = (cellsInRow == 1)
                            ? CCSize(totalWidth, cellHeight)
                            : CCSize(cellWidth, cellHeight);

      auto cell = StageRangeCell::create(&range, m_level, cellSize, rowExpanded);
      cell->ignoreAnchorPointForPosition(true);
      cell->setDisabled(isLocked);

      cell->onExpandChanged = [this, row](StageRangeCell *target, bool expanded)
      {
        for (auto &anotherCell : CCArrayExt<StageRangeCell *>(row->getChildren()))
        {
          if (anotherCell != target)
            anotherCell->setExpanded(expanded);

          if (m_expandedRanges)
            (*m_expandedRanges)[anotherCell->getRangeId()] = expanded;
        }

        row->updateLayout();
      };

      if (!currentRangeId.empty() && range.id == currentRangeId)
        currentRow = row;

      if (!focusRangeId.empty() && range.id == focusRangeId)
      {
        focusRow = row;
        focusCell = cell;
      }

      row->addChild(cell);
      row->updateLayout();
    }

    m_content->addChild(row);
  }

  // ! --- Hidden runs counter --- !
  if (hiddenCount > 0 && !visibleRanges.empty())
  {
    auto footer = CCNode::create();
    footer->setContentSize({totalWidth, 14.f});

    auto label = CCLabelBMFont::create(
        fmt::format(
            "{} completed {} hidden",
            hiddenCount,
            hiddenCount == 1 ? "run" : "runs")
            .c_str(),
        "bigFont.fnt");
    label->setScale(.3f);
    label->setOpacity(120);
    label->setPosition(footer->getContentSize() / 2);
    footer->addChild(label);

    m_content->addChild(footer);
  }

  m_content->updateLayout();
  contentLayer->updateLayout();

  // ! --- Scroll position --- !
  const float contentHeight = contentLayer->getContentHeight();
  const float maxDistance = std::max(0.f, contentHeight - viewHeight);
  float distance = 0.f;

  if (keepScroll)
  {
    distance = distanceFromTop;
  }
  else if (auto *targetRow = focusRow ? focusRow : currentRow)
  {
    // Distance from the top of the content to the top of the row, with a small margin
    const float rowTop =
        m_content->boundingBox().getMinY() + targetRow->boundingBox().getMaxY();

    distance = contentHeight - rowTop - gap;
  }

  distance = std::clamp(distance, 0.f, maxDistance);
  contentLayer->setPositionY(viewHeight - contentHeight + distance);

  // After the tab switch settles
  if (focusCell)
    focusCell->flash(.25f);
}

void StagePage::onGoToCurrent(CCObject *)
{
  if (onGoToCurrentStage)
    onGoToCurrentStage();
}
