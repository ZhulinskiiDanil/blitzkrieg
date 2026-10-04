#include "StageListLayer.hpp"

#include <algorithm>

namespace
{
  // First keybind of a keybind setting, empty when nothing is bound
  std::string getKeybindText(const char *settingKey)
  {
    auto keybinds = Mod::get()->getSettingValue<std::vector<Keybind>>(settingKey);
    return keybinds.empty() ? std::string() : keybinds.front().toString();
  }

  // Bottom center of a node in the space of another node
  CCPoint getBottomCenterIn(CCNode *node, CCNode *space)
  {
    const auto box = node->boundingBox();
    const auto world = node->getParent()->convertToWorldSpace({box.getMidX(), box.getMinY()});
    return space->convertToNodeSpace(world);
  }
}

StageListLayer *StageListLayer::create(
    GJGameLevel *level,
    const CCSize &contentSize)
{
  auto *ret = new StageListLayer();
  if (ret && ret->init(level, contentSize))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

// Pages are built by the first reload(), after the options are set
bool StageListLayer::init(
    GJGameLevel *level,
    const CCSize &contentSize)
{
  if (!CCLayer::init())
    return false;

  m_contentSize = contentSize;
  m_level = level;

  if (auto *profile = GlobalStore::get()->getProfileByLevel(m_level))
    m_profileId = profile->id;

  this->setContentSize(m_contentSize);

  const int total = getStagesCount();

  if (total == 0)
    return true;

  // ! --- Start from the stage the player is on, or the last one --- !
  const int progressIndex = getProgressIndex();
  m_stageIndex = progressIndex >= 0 ? progressIndex : total - 1;

  // ! --- BG --- !
  RectNode *bg = RectNode::create(contentSize, ccc4FFromccc4B({30, 30, 30, 255}), 8);
  bg->ignoreAnchorPointForPosition(false);
  bg->setAnchorPoint({0.5f, 0.5f});
  bg->setPosition(contentSize / 2);
  bg->setZOrder(-1);
  this->addChild(bg);

  // ! --- Pages --- !
  auto stencil = CCLayerColor::create({255, 255, 255, 255}, contentSize.width, contentSize.height);
  m_clip = CCClippingNode::create(stencil);
  this->addChild(m_clip);

  for (auto &page : m_pages)
  {
    page = StagePage::create(m_level, contentSize, &m_expandedRanges);
    page->onGoToCurrentStage = [this]()
    {
      const int progress = getProgressIndex();
      switchStage(progress >= 0 ? progress : getStagesCount() - 1);
    };

    m_clip->addChild(page);
  }

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

  // ! --- Navigation --- !
  createArrows();
  createDots();

  // ! --- Events --- !
  // A run was checked or unchecked: lock states, the hidden runs counter,
  // stage dots and the header follow the new progress
  m_listenerStageRangesChanged = StageRangesChangedEvent().listen(
      [this]()
      {
        queueReload();
        return ListenerResult::Propagate;
      });

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

// ! --- Data --- !

Profile *StageListLayer::getProfile() const
{
  if (m_profileId.empty())
    return nullptr;

  return GlobalStore::get()->getProfileById(m_profileId);
}

std::vector<Stage *> StageListLayer::getStages() const
{
  auto *profile = getProfile();

  if (!profile)
    return {};

  return getConsideredStages(profile->data.stages);
}

Stage *StageListLayer::getCurrentStage() const
{
  const auto stages = getStages();

  if (m_stageIndex < 0 || m_stageIndex >= static_cast<int>(stages.size()))
    return nullptr;

  return stages[m_stageIndex];
}

int StageListLayer::getProgressIndex() const
{
  const auto stages = getStages();

  for (std::size_t i = 0; i < stages.size(); ++i)
  {
    if (!isStageDeepChecked(*stages[i]))
      return static_cast<int>(i);
  }

  return -1;
}

// ! --- Pages --- !

void StageListLayer::buildPage(StagePage *page, int stageIndex, bool keepScroll)
{
  if (!page)
    return;

  const auto stages = getStages();
  const bool inRange = stageIndex >= 0 && stageIndex < static_cast<int>(stages.size());

  page->build(
      inRange ? stages[stageIndex] : nullptr,
      stageIndex,
      getProgressIndex(),
      m_options,
      keepScroll);
}

void StageListLayer::layoutPages()
{
  const float width = m_contentSize.width;

  for (int i = 0; i < 3; ++i)
  {
    auto *page = m_pages[i];

    if (!page)
      continue;

    page->stopAllActions();
    page->setPosition({(i - 1) * width, 0.f});
    // Hidden pages do not take touches
    page->setVisible(i == 1);
  }
}

void StageListLayer::reload(bool keepScroll)
{
  if (!m_pages[1])
    return;

  if (m_isSliding)
  {
    // Pages are moving, rebuild each one in place
    for (auto *page : m_pages)
    {
      if (page->getStageIndex() >= 0)
        buildPage(page, page->getStageIndex(), true);
    }
  }
  else
  {
    for (int i = 0; i < 3; ++i)
    {
      const int stageIndex = m_stageIndex - 1 + i;
      const bool keep = keepScroll && m_pages[i]->getStageIndex() == stageIndex;

      buildPage(m_pages[i], stageIndex, keep);
    }

    layoutPages();
  }

  updateNavigation();
}

void StageListLayer::queueReload()
{
  if (m_reloadQueued)
    return;

  m_reloadQueued = true;

  geode::queueInMainThread(
      [self = Ref<StageListLayer>(this)]()
      {
        self->m_reloadQueued = false;

        // The popup was closed before the reload ran
        if (!self->getParent())
          return;

        self->reload(true);
        self->sendStageSwitched();
      });
}

void StageListLayer::setSortBy(StageListSortBy sortBy)
{
  m_options.sortBy = sortBy;
}

void StageListLayer::setRunsVisabilityForCompleted(bool visible)
{
  m_options.hideCompletedRuns = visible;
}

// ! --- Switching --- !

void StageListLayer::sendStageSwitched()
{
  StageSwitchedEvent().send(m_stageIndex, getStagesCount(), getCurrentStage());
}

void StageListLayer::switchStage(int index)
{
  const int total = getStagesCount();

  if (total < 2 || !m_pages[1])
    return;

  // A new switch finishes the previous slide right away
  if (m_isSliding)
  {
    this->stopAllActions();
    onSlideFinished();
  }

  index = std::clamp(index, 0, total - 1);

  if (index == m_stageIndex)
    return;

  const int direction = index > m_stageIndex ? 1 : -1;
  auto *outgoing = m_pages[1];
  auto *incoming = m_pages[1 + direction];

  // A jump over several stages puts the target into the neighbor slot first
  if (incoming->getStageIndex() != index)
    buildPage(incoming, index, false);

  m_stageIndex = index;
  m_slideDirection = direction;
  m_isSliding = true;

  sendStageSwitched();
  updateNavigation();

  const float width = m_contentSize.width;

  incoming->setVisible(true);
  incoming->setPosition({direction * width, 0.f});

  outgoing->runAction(CCEaseInOut::create(
      CCMoveTo::create(SLIDE_DURATION, {-direction * width, 0.f}), 2.f));
  incoming->runAction(CCEaseInOut::create(
      CCMoveTo::create(SLIDE_DURATION, {0.f, 0.f}), 2.f));

  this->runAction(CCSequence::createWithTwoActions(
      CCDelayTime::create(SLIDE_DURATION),
      CCCallFunc::create(this, callfunc_selector(StageListLayer::onSlideFinished))));
}

void StageListLayer::onSlideFinished()
{
  if (!m_isSliding)
    return;

  m_isSliding = false;

  // Rotate the ring, the incoming page becomes the current one
  if (m_slideDirection > 0)
    std::rotate(m_pages.begin(), m_pages.begin() + 1, m_pages.end());
  else
    std::rotate(m_pages.begin(), m_pages.begin() + 2, m_pages.end());

  // Neighbors that do not show the right stage are rebuilt,
  // the page that just left keeps its scroll when it is still a neighbor
  for (int i : {0, 2})
  {
    const int stageIndex = m_stageIndex - 1 + i;

    if (m_pages[i]->getStageIndex() != stageIndex)
      buildPage(m_pages[i], stageIndex, false);
  }

  layoutPages();
}

// ! --- Navigation --- !

void StageListLayer::createArrows()
{
  if (getStagesCount() < 2)
    return;

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
  auto buttonLeft = CCMenuItemSpriteExtra::create(
      btnLeftSpr,
      this,
      menu_selector(StageListLayer::onPrevStageBtn));
  m_buttonMenuLeft->addChild(buttonLeft);
  m_buttonMenuLeft->updateLayout();
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
  auto buttonRight = CCMenuItemSpriteExtra::create(
      btnRightSpr,
      this,
      menu_selector(StageListLayer::onNextStageBtn));
  m_buttonMenuRight->addChild(buttonRight);
  m_buttonMenuRight->updateLayout();
  this->addChild(m_buttonMenuRight);

  // ! --- Keybind hints under the arrows --- !
  auto createKeybindLabel = [this](const char *settingKey, CCNode *button)
  {
    const auto text = getKeybindText(settingKey);

    if (text.empty())
      return static_cast<CCLabelBMFont *>(nullptr);

    auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    label->setScale(.3f);
    label->setOpacity(120);
    label->setAnchorPoint({.5f, 1.f});
    label->setPosition(getBottomCenterIn(button, this) - CCPoint{0.f, 3.f});
    this->addChild(label);

    return label;
  };

  m_keybindLabelLeft = createKeybindLabel("prev-stage-keybind", buttonLeft);
  m_keybindLabelRight = createKeybindLabel("next-stage-keybind", buttonRight);
}

void StageListLayer::createDots()
{
  const int total = getStagesCount();

  if (total < 2)
    return;

  m_dotsMenu = CCMenu::create();
  m_dotsMenu->setPosition({m_contentSize.width / 2, DOTS_OFFSET_Y});
  this->addChild(m_dotsMenu);

  for (int i = 0; i < total; ++i)
  {
    StageDot dot;

    // The dot is thin, the button around it is easier to hit
    dot.hitArea = CCNode::create();
    dot.rect = RectNode::create({1.f, 1.f});
    dot.hitArea->ignoreAnchorPointForPosition(true);
    dot.hitArea->addChild(dot.rect);

    dot.item = CCMenuItemSpriteExtra::create(
        dot.hitArea,
        this,
        menu_selector(StageListLayer::onStageDot));
    dot.item->setTag(i);
    dot.item->m_scaleMultiplier = 1.15f;

    m_dotsMenu->addChild(dot.item);
    m_dots.push_back(dot);
  }
}

// Arrows and dots are created once and only updated here,
// so their buttons are never destroyed inside their own callbacks
void StageListLayer::updateNavigation()
{
  const int total = getStagesCount();

  // ! --- Arrows --- !
  const bool hasPrev = m_stageIndex > 0;
  const bool hasNext = m_stageIndex + 1 < total;

  if (m_buttonMenuLeft)
    m_buttonMenuLeft->setVisible(hasPrev);
  if (m_keybindLabelLeft)
    m_keybindLabelLeft->setVisible(hasPrev);
  if (m_buttonMenuRight)
    m_buttonMenuRight->setVisible(hasNext);
  if (m_keybindLabelRight)
    m_keybindLabelRight->setVisible(hasNext);

  // ! --- Dots --- !
  if (m_dots.empty())
    return;

  constexpr float DOT_WIDTH = 8.f;
  constexpr float ACTIVE_DOT_WIDTH = 18.f;
  constexpr float DOT_HEIGHT = 4.f;
  constexpr float DOT_GAP = 4.f;
  constexpr float HIT_HEIGHT = 14.f;

  const ccColor3B completedColor{99, 224, 110};
  const ccColor3B progressColor{255, 220, 90};
  const ccColor3B lockedColor{90, 90, 90};

  const int count = static_cast<int>(m_dots.size());
  const int progressIndex = getProgressIndex();

  // Many stages shrink the dots to fit under the list
  const float naturalWidth = (count - 1) * (DOT_WIDTH + DOT_GAP) + ACTIVE_DOT_WIDTH + DOT_GAP;
  const float maxWidth = m_contentSize.width - 40.f;
  const float scale = std::min(1.f, maxWidth / naturalWidth);

  float x = -naturalWidth * scale / 2;

  for (int i = 0; i < count; ++i)
  {
    auto &dot = m_dots[i];
    const bool isShown = i == m_stageIndex;

    const float width = (isShown ? ACTIVE_DOT_WIDTH : DOT_WIDTH) * scale;
    const float slotWidth = width + DOT_GAP * scale;
    const CCSize hitSize{slotWidth, HIT_HEIGHT};

    ccColor3B color = lockedColor;

    if (progressIndex < 0 || i < progressIndex)
      color = completedColor;
    else if (i == progressIndex)
      color = progressColor;

    auto color4F = ccc4FFromccc3B(color);
    color4F.a = isShown ? 1.f : .55f;

    dot.rect->setSize({width, DOT_HEIGHT});
    dot.rect->setColor(color4F);
    dot.rect->setPosition({(slotWidth - width) / 2, (HIT_HEIGHT - DOT_HEIGHT) / 2});

    dot.hitArea->setContentSize(hitSize);
    dot.item->setContentSize(hitSize);
    dot.item->setPosition({x + slotWidth / 2, 0.f});

    x += slotWidth;
  }
}

void StageListLayer::onPrevStageBtn(CCObject *sender)
{
  onPrevStage();
}

void StageListLayer::onNextStageBtn(CCObject *sender)
{
  onNextStage();
}

void StageListLayer::onStageDot(CCObject *sender)
{
  if (auto *node = typeinfo_cast<CCNode *>(sender))
    switchStage(node->getTag());
}

void StageListLayer::onPrevStage()
{
  switchStage(m_stageIndex - 1);
}

void StageListLayer::onNextStage()
{
  switchStage(m_stageIndex + 1);
}
