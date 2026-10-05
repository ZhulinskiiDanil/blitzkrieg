#include "SessionLayer.hpp"

#include <algorithm>
#include <fmt/chrono.h>

#include "../../../../events/SessionChangedEvent.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../ui/Label.hpp"
#include "../../../../utils/formatTimePlayed.hpp"

namespace
{
  const ccColor4B ACCENT_COLOR{255, 0, 82, 255};
  const ccColor4B BUTTON_COLOR{32, 32, 32, 255};
  const ccColor4B CHIP_COLOR{36, 36, 36, 255};
  const ccColor4B LIST_COLOR{30, 30, 30, 255};

  const char *LEVEL_FILTER_SAVE_KEY = "session-this-level-only";

  bool isCounted(AttemptOutcome outcome)
  {
    return outcome == AttemptOutcome::Counted ||
           outcome == AttemptOutcome::CountedChecked ||
           outcome == AttemptOutcome::RunPassed ||
           outcome == AttemptOutcome::StageClosed;
  }

  bool isPassed(AttemptOutcome outcome)
  {
    return outcome == AttemptOutcome::RunPassed ||
           outcome == AttemptOutcome::StageClosed;
  }

  // Oct 05 18:40
  std::string formatSince(std::time_t time)
  {
    if (time <= 0)
      return "-";

    return fmt::format("{:%b %d %H:%M}", geode::localtime(time));
  }
}

SessionLayer *SessionLayer::create(GJGameLevel *level, const CCSize &size)
{
  auto ret = new SessionLayer();
  if (ret && ret->init(level, size))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool SessionLayer::init(GJGameLevel *level, const CCSize &size)
{
  if (!CCLayer::init())
    return false;

  m_size = size;
  this->setContentSize(size);

  if (level)
  {
    m_levelId = level->m_levelID
                    ? utils::numToString(level->m_levelID.value())
                    : utils::numToString(EditorIDs::getID(level));
  }

  m_thisLevelOnly = Mod::get()->getSavedValue<bool>(LEVEL_FILTER_SAVE_KEY, true);

  // ! --- Layout, top to bottom --- !
  const float controlsY = m_size.height - TOP_PADDING - CONTROLS_HEIGHT / 2;
  const float listTop = controlsY - CONTROLS_HEIGHT / 2 - ROW_GAP;

  drawControls(controlsY);
  drawList(listTop);
  rebuildList(false);

  m_sessionChangedListener = SessionChangedEvent().listen(
      [this]()
      {
        queueRebuild(true);
        return ListenerResult::Propagate;
      });

  return true;
}

// ! --- Data --- !

std::vector<SessionAttempt const *> SessionLayer::getVisibleAttempts() const
{
  auto const &attempts = SessionStore::get()->getAttempts();
  const bool filter = m_thisLevelOnly && !m_levelId.empty();

  std::vector<SessionAttempt const *> result;
  result.reserve(attempts.size());

  for (auto it = attempts.rbegin(); it != attempts.rend(); ++it)
  {
    if (!filter || it->levelId == m_levelId)
      result.push_back(&*it);
  }

  return result;
}

// ! --- Controls: summary on the left, buttons on the right --- !

SessionLayer::PillButton SessionLayer::createPill(
    CCMenu *menu,
    const char *text,
    float width,
    float x,
    SEL_MenuHandler selector)
{
  const CCSize size{width, CONTROLS_HEIGHT};

  auto content = CCNode::create();
  content->setContentSize(size);

  auto bg = RectNode::create(size, ccc4FFromccc4B(BUTTON_COLOR), size.height / 2);
  content->addChild(bg);

  auto label = CCLabelBMFont::create(text, "bigFont.fnt");
  label->limitLabelWidth(width - 10.f, .28f, .1f);
  label->setPosition(size / 2);
  content->addChild(label);

  auto item = CCMenuItemSpriteExtra::create(content, this, selector);
  item->m_scaleMultiplier = 1.05f;
  item->setPosition({x + width / 2, 0.f});
  menu->addChild(item);

  return {item, bg};
}

void SessionLayer::drawControls(float y)
{
  const float gap = 4.f;
  const float widths[] = {58.f, 46.f, 42.f};
  const float buttonsWidth = widths[0] + widths[1] + widths[2] + gap * 2;
  const float buttonsLeft = m_size.width - SIDE_PADDING - buttonsWidth;

  auto menu = CCMenu::create();
  menu->setPosition({buttonsLeft, y});
  this->addChild(menu);

  float x = 0.f;

  m_levelFilterButton = createPill(menu, "This level", widths[0], x, menu_selector(SessionLayer::onLevelFilter));
  x += widths[0] + gap;

  createPill(menu, "Export", widths[1], x, menu_selector(SessionLayer::onExport));
  x += widths[1] + gap;

  auto reset = createPill(menu, "Reset", widths[2], x, menu_selector(SessionLayer::onReset));

  // Reset deletes the log, its text is red
  if (auto *content = reset.item->getNormalImage())
  {
    for (auto *child : CCArrayExt<CCNode *>(content->getChildren()))
    {
      if (auto *label = typeinfo_cast<CCLabelBMFont *>(child))
        label->setColor({253, 106, 106});
    }
  }

  updateLevelFilterButton();

  // ! Summary chips fill the room left of the buttons
  m_summary = CCNode::create();
  m_summary->setPosition({SIDE_PADDING, y});
  m_summary->setContentSize({buttonsLeft - 8.f - SIDE_PADDING, 0.f});
  this->addChild(m_summary);
}

void SessionLayer::updateLevelFilterButton()
{
  if (m_levelFilterButton.bg)
  {
    m_levelFilterButton.bg->setColor(ccc4FFromccc4B(
        m_thisLevelOnly ? ACCENT_COLOR : BUTTON_COLOR));
  }
}

void SessionLayer::updateSummary(std::vector<SessionAttempt const *> const &attempts)
{
  if (!m_summary)
    return;

  m_summary->removeAllChildrenWithCleanup(true);
  m_summary->setScale(1.f);

  int counted = 0;
  int passed = 0;
  float time = 0.f;

  for (auto const *attempt : attempts)
  {
    if (isCounted(attempt->outcome))
      counted++;

    if (isPassed(attempt->outcome))
      passed++;

    time += std::max(0.f, attempt->duration);
  }

  const std::string chips[] = {
      fmt::format("{} <small>attempts</small>", attempts.size()),
      fmt::format("{} <small>counted</small>", counted),
      fmt::format("{} <small>passed</small>", passed),
      formatTimePlayed(time),
      fmt::format("<small>since</small> {}", formatSince(SessionStore::get()->getStartedAt())),
  };

  const float chipPadding = 5.f;
  const float chipGap = 4.f;
  float x = 0.f;

  for (auto const &text : chips)
  {
    auto label = UILabel::create(text, "bigFont.fnt", .26f);
    const float width = label->getContentWidth() + chipPadding * 2;

    auto chip = RectNode::create({width, CONTROLS_HEIGHT}, ccc4FFromccc4B(CHIP_COLOR), CONTROLS_HEIGHT / 2);
    chip->setPosition({x, -CONTROLS_HEIGHT / 2});
    m_summary->addChild(chip);

    label->setAnchorPoint({0.f, .5f});
    label->setPosition({x + chipPadding, 0.f});
    m_summary->addChild(label);

    x += width + chipGap;
  }

  // Too many digits: shrink from the left edge
  const float maxWidth = m_summary->getContentWidth();
  const float width = x - chipGap;

  if (width > maxWidth && width > 0.f)
    m_summary->setScale(maxWidth / width);
}

// ! --- List --- !

void SessionLayer::drawList(float top)
{
  const CCSize listSize{m_size.width - SIDE_PADDING * 2, top - BOTTOM_PADDING};
  const CCPoint listCenter{SIDE_PADDING + listSize.width / 2, BOTTOM_PADDING + listSize.height / 2};

  auto background = RectNode::create(listSize, ccc4FFromccc4B(LIST_COLOR), 8.f);
  background->ignoreAnchorPointForPosition(false);
  background->setAnchorPoint({.5f, .5f});
  background->setPosition(listCenter);
  this->addChild(background, -1);

  m_scroll = ScrollLayer::create({listSize.width - 8.f, listSize.height - 8.f});
  m_scroll->setPosition({SIDE_PADDING + 4.f, BOTTOM_PADDING + 4.f});
  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(4.f)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));
  this->addChild(m_scroll);

  // ! --- Borders, the same as the other lists --- !
  auto borders = ListBorders::create();
  borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f);
  borders->setContentSize({listSize.width, listSize.height - 3.f});
  borders->setAnchorPoint({.5f, .5f});
  borders->setPosition(listCenter - CCPoint{0.f, .5f});
  borders->updateLayout();
  this->addChild(borders);

  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
    child->setColor(ccc3(50, 50, 50));

  // ! --- Empty state, the texts are set by rebuildList --- !
  m_emptyState = CCNode::create();
  m_emptyState->setPosition(listCenter);
  m_emptyState->setVisible(false);
  this->addChild(m_emptyState, 1);

  auto title = CCLabelBMFont::create("", "bigFont.fnt");
  title->setID("title");
  title->setScale(.45f);
  title->setOpacity(170);
  title->setPosition({0.f, 8.f});
  m_emptyState->addChild(title);

  auto hint = CCLabelBMFont::create("", "bigFont.fnt");
  hint->setID("hint");
  hint->setScale(.28f);
  hint->setOpacity(110);
  hint->setPosition({0.f, -8.f});
  m_emptyState->addChild(hint);
}

void SessionLayer::rebuildList(bool keepScroll)
{
  if (!m_scroll)
    return;

  auto *contentLayer = m_scroll->m_contentLayer;
  const float viewHeight = m_scroll->getContentHeight();

  // Content layer is at (viewHeight - contentHeight) when scrolled to the top
  const float distanceFromTop =
      contentLayer->getPositionY() - (viewHeight - contentLayer->getContentHeight());

  contentLayer->removeAllChildrenWithCleanup(true);

  const auto attempts = getVisibleAttempts();
  updateSummary(attempts);

  // ! --- Empty state --- !
  m_emptyState->setVisible(attempts.empty());

  if (attempts.empty())
  {
    const bool filtered = !SessionStore::get()->getAttempts().empty();

    if (auto *title = typeinfo_cast<CCLabelBMFont *>(m_emptyState->getChildByID("title")))
      title->setString(filtered ? "No attempts on this level" : "No attempts yet");

    if (auto *hint = typeinfo_cast<CCLabelBMFont *>(m_emptyState->getChildByID("hint")))
      hint->setString(filtered ? "Turn off \"This level\" to see every level" : "Every attempt of a tracked level shows up here");
  }

  // ! --- Cells, newest first --- !
  const float width = m_scroll->getContentWidth();
  const std::size_t count = std::min(attempts.size(), m_shownCount);

  for (std::size_t i = 0; i < count; ++i)
  {
    auto const *attempt = attempts[i];

    auto cell = SessionAttemptCell::create(*attempt, width, m_expanded.contains(attempt->id));
    cell->onExpandChanged = [this](std::uint64_t id, bool expanded)
    {
      if (expanded)
        m_expanded.insert(id);
      else
        m_expanded.erase(id);

      queueRebuild(true);
    };

    contentLayer->addChild(cell);
  }

  // ! --- Show more --- !
  if (attempts.size() > count)
  {
    auto footer = CCNode::create();
    footer->setContentSize({width, 24.f});

    auto button = CCMenuItemSpriteExtra::create(
        ButtonSprite::create(fmt::format("Show more ({} left)", attempts.size() - count).c_str()),
        this,
        menu_selector(SessionLayer::onShowMore));
    button->setScale(.5f);
    button->m_baseScale = .5f;

    auto menu = CCMenu::createWithItem(button);
    menu->setPosition(footer->getContentSize() / 2);
    footer->addChild(menu);

    contentLayer->addChild(footer);
  }

  contentLayer->updateLayout();

  // ! --- Scroll position --- !
  const float contentHeight = contentLayer->getContentHeight();
  const float maxDistance = std::max(0.f, contentHeight - viewHeight);
  const float distance = keepScroll ? std::clamp(distanceFromTop, 0.f, maxDistance) : 0.f;

  contentLayer->setPositionY(viewHeight - contentHeight + distance);
}

void SessionLayer::queueRebuild(bool keepScroll)
{
  if (m_rebuildQueued)
    return;

  m_rebuildQueued = true;

  geode::queueInMainThread(
      [self = Ref<SessionLayer>(this), keepScroll]()
      {
        self->m_rebuildQueued = false;

        // The popup was closed before the rebuild ran
        if (!self->getParent())
          return;

        self->rebuildList(keepScroll);
      });
}

// ! --- Buttons --- !

void SessionLayer::onLevelFilter(CCObject *)
{
  m_thisLevelOnly = !m_thisLevelOnly;
  m_shownCount = PAGE_SIZE;

  Mod::get()->setSavedValue(LEVEL_FILTER_SAVE_KEY, m_thisLevelOnly);

  updateLevelFilterButton();
  queueRebuild(false);
}

void SessionLayer::onShowMore(CCObject *)
{
  m_shownCount += PAGE_SIZE;
  queueRebuild(true);
}

void SessionLayer::onExport(CCObject *)
{
  auto result = SessionStore::get()->exportToFile();

  if (result.isErr())
  {
    Notification::create(
        fmt::format("Export failed: {}", result.unwrapErr()),
        NotificationIcon::Error)
        ->show();
    return;
  }

  const auto path = result.unwrap();

  Notification::create("Session log exported", NotificationIcon::Success)->show();
  file::openFolder(path.parent_path());
}

void SessionLayer::onReset(CCObject *)
{
  const auto count = SessionStore::get()->getAttempts().size();

  if (count == 0)
    return;

  geode::createQuickPopup(
      "Reset Session",
      fmt::format(
          "All <cy>{} attempts</c> of the session log will be <cr>deleted</c>.\n"
          "Profiles and their stats stay as they are.",
          count),
      "Cancel",
      "Reset",
      [self = Ref<SessionLayer>(this)](auto, bool confirmed)
      {
        if (!confirmed)
          return;

        self->m_expanded.clear();
        self->m_shownCount = PAGE_SIZE;
        SessionStore::get()->reset();
      });
}
