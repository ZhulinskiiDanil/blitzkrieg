#include "SessionDaysView.hpp"

#include <algorithm>
#include <cmath>
#include <fmt/chrono.h>

#include "../../../../utils/dateKey.hpp"
#include "../../../../utils/formatTimePlayed.hpp"

namespace
{
  constexpr std::time_t DAY = 24 * 60 * 60;

  const ccColor3B EMPTY_COLOR{48, 48, 48};
  const ccColor3B LOW_COLOR{80, 42, 56};
  const ccColor3B ACCENT_COLOR{255, 0, 82};
  const ccColor4B CHIP_COLOR{36, 36, 36, 255};

  ccColor3B lerpColor(ccColor3B from, ccColor3B to, float t)
  {
    auto lerp = [t](GLubyte a, GLubyte b)
    {
      return static_cast<GLubyte>(std::round(a + (b - a) * t));
    };

    return {lerp(from.r, to.r), lerp(from.g, to.g), lerp(from.b, to.b)};
  }

  // Four steps of the accent, by the share of the busiest day
  ccColor3B getHeatColor(int attempts, int maxAttempts)
  {
    if (attempts <= 0 || maxAttempts <= 0)
      return EMPTY_COLOR;

    const float ratio = static_cast<float>(attempts) / maxAttempts;
    const float step = std::ceil(ratio * 4.f) / 4.f;

    return lerpColor(LOW_COLOR, ACCENT_COLOR, (step - .25f) / .75f);
  }

  std::string formatDay(std::time_t time)
  {
    return fmt::format("{:%a, %b %d}", geode::localtime(time));
  }
}

void addHistory(std::map<std::string, DayStats> &into, std::map<std::string, DayStats> const &from)
{
  for (auto const &[key, day] : from)
  {
    auto &target = into[key];
    target.attempts += day.attempts;
    target.timePlayed += day.timePlayed;
    target.runsPassed += day.runsPassed;
    target.stagesClosed += day.stagesClosed;
    target.bestFromZero = std::max(target.bestFromZero, day.bestFromZero);
  }
}

SessionDaysView *SessionDaysView::create(
    CCSize const &size,
    std::map<std::string, DayStats> history,
    bool showBestFromZero)
{
  auto ret = new SessionDaysView();
  if (ret && ret->init(size, std::move(history), showBestFromZero))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool SessionDaysView::init(
    CCSize const &size,
    std::map<std::string, DayStats> history,
    bool showBestFromZero)
{
  if (!CCNode::init())
    return false;

  m_size = size;
  m_history = std::move(history);
  m_showBestFromZero = showBestFromZero;

  this->setContentSize(size);

  // ! --- Layout, top to bottom --- !
  const float chipsTop = m_size.height;
  const float gridTop = chipsTop - CHIPS_HEIGHT - GAP;
  const float infoY = INFO_HEIGHT / 2;

  drawChips(chipsTop);
  drawGrid(gridTop, INFO_HEIGHT + GAP);
  drawInfo(infoY);

  // Today is shown until another day is picked
  select(static_cast<int>(m_cells.size()) - 1);

#ifdef GEODE_IS_DESKTOP
  this->scheduleUpdate();
#endif

  return true;
}

DayStats SessionDaysView::getDay(std::string const &key) const
{
  auto it = m_history.find(key);
  return it != m_history.end() ? it->second : DayStats{};
}

// ! --- Chips: today, streak, active days, busiest day --- !

void SessionDaysView::drawChips(float top)
{
  const auto todayNoon = getLocalNoon(std::time(nullptr));
  const auto today = getDay(getDateKey(todayNoon));

  // ! A streak may still go on when nothing was played today yet
  int streak = 0;

  for (std::time_t day = today.attempts > 0 ? todayNoon : todayNoon - DAY;; day -= DAY)
  {
    if (getDay(getDateKey(day)).attempts <= 0)
      break;

    ++streak;
  }

  int activeDays = 0;
  std::string busiestKey;
  int busiestAttempts = 0;

  for (auto const &[key, day] : m_history)
  {
    if (day.attempts <= 0)
      continue;

    ++activeDays;

    if (day.attempts > busiestAttempts)
    {
      busiestAttempts = day.attempts;
      busiestKey = key;
    }
  }

  std::vector<std::string> chips = {
      fmt::format("{} <small>today</small>   {}", today.attempts, formatTimePlayed(today.timePlayed)),
      fmt::format("{} <small>{} streak</small>", streak, streak == 1 ? "day" : "days"),
      fmt::format("{} <small>active {}</small>", activeDays, activeDays == 1 ? "day" : "days"),
  };

  if (busiestAttempts > 0)
    chips.push_back(fmt::format("<small>busiest</small> {} <small>on {}</small>", busiestAttempts, busiestKey));

  auto row = CCNode::create();
  row->setPosition({0.f, top - CHIPS_HEIGHT / 2});
  this->addChild(row);

  const float chipPadding = 5.f;
  const float chipGap = 4.f;
  float x = 0.f;

  for (auto const &text : chips)
  {
    auto label = UILabel::create(text, "bigFont.fnt", .26f);
    const float width = label->getContentWidth() + chipPadding * 2;

    auto chip = RectNode::create({width, CHIPS_HEIGHT}, ccc4FFromccc4B(CHIP_COLOR), CHIPS_HEIGHT / 2);
    chip->setPosition({x, -CHIPS_HEIGHT / 2});
    row->addChild(chip);

    label->setPosition({x + chipPadding, 0.f});
    label->setAnchorPoint({0, .5f});
    row->addChild(label);

    x += width + chipGap;
  }

  // Too many digits: shrink from the left edge
  const float width = x - chipGap;

  if (width > m_size.width && width > 0.f)
    row->setScale(m_size.width / width);
}

// ! --- Heatmap: a column per week, Monday on top --- !

void SessionDaysView::drawGrid(float top, float bottom)
{
  const float gridHeight = top - bottom - MONTHS_HEIGHT;
  const float byWidth = (m_size.width - WEEKDAYS_WIDTH - CELL_GAP * (WEEKS - 1)) / WEEKS;
  const float byHeight = (gridHeight - CELL_GAP * 6) / 7;

  m_cellSize = std::max(3.f, std::min(byWidth, byHeight));

  const float step = m_cellSize + CELL_GAP;
  const float width = WEEKDAYS_WIDTH + step * WEEKS - CELL_GAP;
  const float height = step * 7 - CELL_GAP;

  // Centered in the free room
  m_grid = CCNode::create();
  m_grid->setContentSize({width, height});
  m_grid->setPosition({(m_size.width - width) / 2, bottom + (gridHeight - height) / 2});
  this->addChild(m_grid);

  // ! --- Days from the Monday 25 weeks ago to today --- !
  const auto todayNoon = getLocalNoon(std::time(nullptr));
  const auto start = todayNoon - (getWeekdayFromMonday(todayNoon) + (WEEKS - 1) * 7) * DAY;

  int maxAttempts = 0;

  for (std::time_t day = start; day <= todayNoon; day += DAY)
    maxAttempts = std::max(maxAttempts, getDay(getDateKey(day)).attempts);

  auto menu = CCMenu::create();
  menu->setPosition({0.f, 0.f});
  m_grid->addChild(menu, 2);

  // Behind the selected cell
  m_selection = RectNode::create({m_cellSize + 2.f, m_cellSize + 2.f}, {1.f, 1.f, 1.f, 1.f}, 2.5f);
  m_selection->setVisible(false);
  m_grid->addChild(m_selection, 0);

  int lastMonth = -1;
  int index = 0;

  for (std::time_t day = start; day <= todayNoon; day += DAY, ++index)
  {
    const int column = index / 7;
    const int row = index % 7;

    const CCPoint position{WEEKDAYS_WIDTH + column * step, (6 - row) * step};
    const auto key = getDateKey(day);
    const auto stats = getDay(key);

    auto rect = RectNode::create(
        {m_cellSize, m_cellSize},
        ccc4FFromccc3B(getHeatColor(stats.attempts, maxAttempts)),
        std::min(2.f, m_cellSize / 3));
    rect->setPosition(position);
    m_grid->addChild(rect, 1);

    // ! Today has a quiet outline
    if (day == todayNoon)
    {
      auto outline = RectNode::create({m_cellSize + 2.f, m_cellSize + 2.f}, ccc4FFromccc3B({120, 120, 120}), 2.5f);
      outline->setPosition(position - CCPoint{1.f, 1.f});
      m_grid->addChild(outline, 0);
    }

    // ! Tap area around the cell
    auto area = CCNode::create();
    area->setContentSize({step, step});

    auto item = CCMenuItemSpriteExtra::create(area, this, menu_selector(SessionDaysView::onCell));
    item->setTag(index);
    item->m_scaleMultiplier = 1.f;
    item->setPosition(position + CCPoint{m_cellSize / 2, m_cellSize / 2});
    menu->addChild(item);

    // ! Month over the first week that starts in it
    if (row == 0)
    {
      const int month = geode::localtime(day).tm_mon;

      // The first column only gets a label when its month lasts a few weeks
      if (month != lastMonth && (lastMonth >= 0 || geode::localtime(day + 14 * DAY).tm_mon == month))
      {
        auto label = CCLabelBMFont::create(fmt::format("{:%b}", geode::localtime(day)).c_str(), "bigFont.fnt");
        label->setScale(.2f);
        label->setOpacity(140);
        label->setAnchorPoint({0.f, 0.f});
        label->setPosition({position.x, height + 2.f});
        m_grid->addChild(label);
      }

      lastMonth = month;
    }

    m_cells.push_back({key, day, position, rect});
  }

  // ! --- Weekday labels --- !
  const std::pair<int, const char *> weekdays[] = {{0, "Mon"}, {2, "Wed"}, {4, "Fri"}};

  for (auto const &[row, text] : weekdays)
  {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(.2f);
    label->setOpacity(140);
    label->setAnchorPoint({1.f, .5f});
    label->setPosition({WEEKDAYS_WIDTH - 4.f, (6 - row) * step + m_cellSize / 2});
    m_grid->addChild(label);
  }
}

// ! --- The numbers of one day --- !

void SessionDaysView::drawInfo(float y)
{
  auto card = RectNode::create({m_size.width, INFO_HEIGHT}, ccc4FFromccc4B(CHIP_COLOR), 6.f);
  card->setPosition({0.f, y - INFO_HEIGHT / 2});
  this->addChild(card);

  m_info = UILabel::create("", "bigFont.fnt", .28f);
  // Shrinks from the left edge
  m_info->setAnchorPoint({0.f, .5f});
  m_info->setPosition({8.f, y});
  this->addChild(m_info);
}

void SessionDaysView::select(int index)
{
  if (index < 0 || index >= static_cast<int>(m_cells.size()) || index == m_selected)
    return;

  m_selected = index;

  auto const &cell = m_cells[index];
  const auto day = getDay(cell.key);

  m_selection->setPosition(cell.position - CCPoint{1.f, 1.f});
  m_selection->setVisible(true);

  // ! --- Text, the parts with nothing are left out --- !
  std::string text = formatDay(cell.noon);

  if (index + 1 == static_cast<int>(m_cells.size()))
    text += " <small>(today)</small>";

  if (day.attempts <= 0)
  {
    text += "   <small>nothing played</small>";
  }
  else
  {
    text += fmt::format(
        "   {} <small>{}</small>   {}",
        day.attempts,
        day.attempts == 1 ? "attempt" : "attempts",
        formatTimePlayed(day.timePlayed));

    if (day.runsPassed > 0)
      text += fmt::format("   {} <small>{} passed</small>", day.runsPassed, day.runsPassed == 1 ? "run" : "runs");

    if (day.stagesClosed > 0)
      text += fmt::format("   {} <small>{} closed</small>", day.stagesClosed, day.stagesClosed == 1 ? "stage" : "stages");

    if (m_showBestFromZero && day.bestFromZero > 0.f)
      text += fmt::format("   <small>best from 0%</small> {:.0f}%", day.bestFromZero);
  }

  m_info->setText(text);
  m_info->setScale(1.f);

  // Long days shrink to fit the card
  const float maxWidth = m_size.width - 16.f;

  if (m_info->getContentWidth() > maxWidth)
    m_info->setScale(maxWidth / m_info->getContentWidth());
}

int SessionDaysView::getCellAt(CCPoint const &gridPoint) const
{
  const float step = m_cellSize + CELL_GAP;
  const float x = gridPoint.x - WEEKDAYS_WIDTH;

  if (x < 0.f || gridPoint.y < 0.f)
    return -1;

  const int column = static_cast<int>(x / step);
  const int row = 6 - static_cast<int>(gridPoint.y / step);

  if (column >= WEEKS || row < 0 || row > 6)
    return -1;

  const int index = column * 7 + row;
  return index < static_cast<int>(m_cells.size()) ? index : -1;
}

void SessionDaysView::onCell(CCObject *sender)
{
  if (auto *node = typeinfo_cast<CCNode *>(sender))
    select(node->getTag());
}

// Hover picks the day with a mouse, a still mouse keeps a tapped day
void SessionDaysView::update(float dt)
{
  if (!m_grid || !nodeIsVisible(this))
    return;

  const auto mousePos = getMousePos();

  if (mousePos.equals(m_lastMousePos))
    return;

  m_lastMousePos = mousePos;

  const int index = getCellAt(m_grid->convertToNodeSpace(mousePos));

  if (index >= 0)
    select(index);
}
