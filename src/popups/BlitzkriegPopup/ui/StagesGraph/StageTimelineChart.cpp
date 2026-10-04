#include "StageTimelineChart.hpp"

#include <algorithm>
#include <cmath>

#include "../../../../utils/formatCompletedAt.hpp"
#include "../../../../utils/formatTimePlayed.hpp"

namespace
{
  const ccColor4B LINE_COLOR{255, 0, 82, 255};
  const ccColor4B FILL_COLOR{255, 0, 82, 38};
  const ccColor4B GOAL_COLOR{99, 224, 110, 130};
  const ccColor4B GRID_COLOR{60, 60, 60, 255};
  const ccColor4B BASELINE_COLOR{90, 90, 90, 255};

  constexpr float LINE_WIDTH = .8f;
  constexpr float DOT_RADIUS = 1.6f;
  constexpr float SELECTED_DOT_RADIUS = 2.8f;

  const ccColor4B FILL_BREAK_COLOR{255, 0, 82, 16};
  const ccColor4B BREAK_MARK_COLOR{150, 150, 150, 255};

  // +5d, +3w, +4mo, +2y
  std::string formatGap(std::time_t seconds)
  {
    const double days = seconds / 86400.0;

    if (days < 14.0)
      return fmt::format("+{}d", static_cast<int>(std::round(days)));
    if (days < 60.0)
      return fmt::format("+{}w", static_cast<int>(std::round(days / 7.0)));
    if (days < 365.0)
      return fmt::format("+{}mo", static_cast<int>(std::round(days / 30.0)));

    return fmt::format("+{}y", static_cast<int>(std::round(days / 365.0)));
  }

  // UILabel has no height, it is centered on its y
  UILabel *createLabel(std::string const &text, float scale, float anchorX)
  {
    auto label = UILabel::create(text, "bigFont.fnt", scale);
    label->ignoreAnchorPointForPosition(false);
    label->setAnchorPoint({anchorX, .5f});
    return label;
  }

  void drawRect(CCDrawNode *node, float left, float right, float top, ccColor4F color)
  {
    if (right - left <= 0.f || top <= 0.f)
      return;

    CCPoint vertices[] = {
        {left, 0.f},
        {right, 0.f},
        {right, top},
        {left, top},
    };

    node->drawPolygon(vertices, 4, color, 0.f, {0.f, 0.f, 0.f, 0.f});
  }

  void drawDashedLine(CCDrawNode *node, CCPoint from, CCPoint to, float width, ccColor4F color)
  {
    const float dash = 3.f;
    const float gap = 3.f;
    const float length = from.getDistance(to);

    if (length <= 0.f)
      return;

    const CCPoint direction = (to - from) / length;

    for (float t = 0.f; t < length; t += dash + gap)
    {
      node->drawSegment(
          from + direction * t,
          from + direction * std::min(t + dash, length),
          width,
          color);
    }
  }
}

StageTimelineChart *StageTimelineChart::create(const CCSize &size)
{
  auto ret = new StageTimelineChart();
  if (ret && ret->init(size))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StageTimelineChart::init(const CCSize &size)
{
  if (!CCLayer::init())
    return false;

  m_size = size;
  m_plotSize = CCSize(size.width - AXIS_LEFT, size.height - AXIS_BOTTOM - PLOT_TOP);
  this->setContentSize(size);

  m_plot = CCNode::create();
  m_plot->setPosition({AXIS_LEFT, AXIS_BOTTOM});
  m_plot->setContentSize(m_plotSize);
  this->addChild(m_plot);

  m_selectedDot = CCDrawNode::create();
  m_plot->addChild(m_selectedDot, 3);

  m_hitMenu = CCMenu::create();
  m_hitMenu->setPosition({0.f, 0.f});
  m_plot->addChild(m_hitMenu, 4);

  // ! --- Tooltip --- !
  m_tooltip = CCNode::create();
  m_tooltip->setVisible(false);
  m_plot->addChild(m_tooltip, 5);

  m_tooltipBg = RectNode::create({1.f, 1.f}, premultiplyAlpha(ccc4FFromccc4B({10, 10, 10, 235})), 4);
  m_tooltip->addChild(m_tooltipBg);

  m_tooltipTitle = createLabel("", .3f, 0.f);
  m_tooltip->addChild(m_tooltipTitle);

  m_tooltipStats = createLabel("", .26f, 0.f);
  m_tooltip->addChild(m_tooltipStats);

  m_tooltipHint = CCLabelBMFont::create(getOpenHint(), "bigFont.fnt");
  m_tooltipHint->setScale(.2f);
  m_tooltipHint->setOpacity(130);
  m_tooltipHint->setAnchorPoint({0.f, .5f});
  m_tooltip->addChild(m_tooltipHint);

#ifdef GEODE_IS_DESKTOP
  this->scheduleUpdate();
#endif

  return true;
}

// ! --- Geometry --- !

float StageTimelineChart::getTimeX(std::time_t time) const
{
  if (m_knots.empty())
    return m_plotSize.width / 2;

  if (time <= m_knots.front().time)
    return m_knots.front().x;

  for (std::size_t i = 0; i + 1 < m_knots.size(); ++i)
  {
    auto const &a = m_knots[i];
    auto const &b = m_knots[i + 1];

    if (time > b.time)
      continue;

    const double t = static_cast<double>(time - a.time) / static_cast<double>(b.time - a.time);
    return a.x + (b.x - a.x) * static_cast<float>(t);
  }

  return m_knots.back().x;
}

// Short pauses keep their real length, long ones get a fixed narrow width
void StageTimelineChart::buildTimeAxis()
{
  m_knots.clear();
  m_breaks.clear();

  std::vector<std::time_t> times;

  for (auto const &point : m_points)
  {
    if (times.empty() || point.time != times.back())
      times.push_back(point.time);
  }

  // Not done yet: the axis goes on to today
  if (const auto now = std::time(nullptr); !m_levelDone && now > times.back())
    times.push_back(now);

  if (times.size() == 1)
  {
    m_knots.push_back({times.front(), m_plotSize.width / 2});
    return;
  }

  const float left = m_plotSize.width * X_MARGIN;
  const float usable = m_plotSize.width * (1.f - X_MARGIN * 2);
  const std::size_t gaps = times.size() - 1;

  std::size_t longGaps = 0;
  double shortSum = 0.0;

  for (std::size_t i = 0; i < gaps; ++i)
  {
    const auto gap = times[i + 1] - times[i];

    if (gap > LONG_GAP)
      longGaps++;
    else
      shortSum += static_cast<double>(gap);
  }

  // Only pauses: they share the width evenly
  float breakWidth = 0.f;

  if (longGaps == gaps)
    breakWidth = usable / gaps;
  else if (longGaps > 0)
    breakWidth = std::min(BREAK_WIDTH, usable * MAX_BREAKS_SHARE / longGaps);

  const float rest = usable - breakWidth * longGaps;

  float x = left;
  m_knots.push_back({times.front(), x});

  for (std::size_t i = 0; i < gaps; ++i)
  {
    const auto gap = times[i + 1] - times[i];
    const bool isLong = gap > LONG_GAP;
    const float width = isLong
                            ? breakWidth
                            : rest * static_cast<float>(static_cast<double>(gap) / shortSum);

    if (isLong)
      m_breaks.push_back({x, x + width, gap});

    x += width;
    m_knots.push_back({times[i + 1], x});
  }
}

CCPoint StageTimelineChart::getPointPosition(Point const &point) const
{
  return {
      getTimeX(point.time),
      m_plotSize.height * point.completed / m_axisMax,
  };
}

// ! --- Data --- !

void StageTimelineChart::setData(std::vector<StageGraphColumn> const &columns)
{
  m_points.clear();
  m_selected = -1;
  m_selectedByHover = false;
  m_totalStages = static_cast<int>(columns.size());

  int undated = 0;
  bool levelDone = !columns.empty();

  for (auto const &column : columns)
  {
    if (column.status != StageGraphStatus::Completed)
    {
      levelDone = false;
      continue;
    }

    // Imported profiles lose their dates
    if (column.completedAt <= 0)
    {
      undated++;
      continue;
    }

    m_points.push_back({
        .time = column.completedAt,
        .stageIndex = column.index,
        .attempts = column.attempts,
        .timePlayed = column.timePlayed,
    });
  }

  std::stable_sort(
      m_points.begin(),
      m_points.end(),
      [](Point const &a, Point const &b)
      { return a.time < b.time; });

  for (std::size_t i = 0; i < m_points.size(); ++i)
    m_points[i].completed = static_cast<int>(i) + 1;

  drawUndatedNote(undated);

  if (m_points.empty())
  {
    drawEmptyState();
    return;
  }

  // ! --- Time axis: first completion to now, or to the last one when the level is done --- !
  m_levelDone = levelDone;
  buildTimeAxis();

  // ! --- Stage axis: 0 to all stages --- !
  m_axisStep = getNiceAxisStep(static_cast<float>(m_totalStages) / GRID_LINES);
  m_axisMax = std::max(m_axisStep, std::ceil(m_totalStages / m_axisStep) * m_axisStep);

  drawAxis();
  drawLine();
  drawHitAreas();
}

// ! --- Drawing --- !

void StageTimelineChart::drawEmptyState()
{
  auto label = CCLabelBMFont::create("No completion dates yet", "bigFont.fnt");
  label->setScale(.4f);
  label->setOpacity(150);
  label->setPosition(m_plotSize / 2);
  m_plot->addChild(label);
}

void StageTimelineChart::drawUndatedNote(int count)
{
  if (count <= 0)
    return;

  auto label = CCLabelBMFont::create(
      fmt::format("{} {} without a date", count, count == 1 ? "stage" : "stages").c_str(),
      "bigFont.fnt");
  label->setScale(.22f);
  label->setOpacity(130);
  label->setAnchorPoint({1.f, .5f});
  label->setPosition({m_plotSize.width, m_plotSize.height + PLOT_TOP / 2});
  m_plot->addChild(label);
}

void StageTimelineChart::drawAxis()
{
  const float width = m_plotSize.width;
  const float height = m_plotSize.height;

  // ! --- Horizontal grid and Y labels --- !
  auto grid = CCDrawNode::create();
  m_plot->addChild(grid, 0);

  const int lines = std::max(1, static_cast<int>(std::round(m_axisMax / m_axisStep)));

  for (int i = 0; i <= lines; ++i)
  {
    const float value = m_axisStep * i;
    const float y = height * value / m_axisMax;

    grid->drawSegment(
        {0.f, y},
        {width, y},
        i == 0 ? .5f : .3f,
        ccc4FFromccc4B(i == 0 ? BASELINE_COLOR : GRID_COLOR));

    auto label = createLabel(std::to_string(static_cast<int>(value)), .25f, 1.f);
    label->setPosition({-5.f, y});
    m_plot->addChild(label);
  }

  // ! --- Goal: every stage completed --- !
  const float goalY = height * m_totalStages / m_axisMax;
  drawDashedLine(grid, {0.f, goalY}, {width, goalY}, .4f, premultiplyAlpha(ccc4FFromccc4B(GOAL_COLOR)));

  auto goalLabel = createLabel("<small>All stages</small>", .22f, 0.f);
  goalLabel->setPosition({2.f, std::min(goalY + 5.f, height + PLOT_TOP / 2)});
  m_plot->addChild(goalLabel);

  // ! --- Breaks: a cut on the baseline and the length of the pause --- !
  const auto markColor = ccc4FFromccc4B(BREAK_MARK_COLOR);

  for (auto const &pause : m_breaks)
  {
    const float center = (pause.left + pause.right) / 2;

    grid->drawSegment({center - 2.5f, -2.f}, {center - .5f, 2.f}, .4f, markColor);
    grid->drawSegment({center + .5f, -2.f}, {center + 2.5f, 2.f}, .4f, markColor);

    auto label = CCLabelBMFont::create(formatGap(pause.duration).c_str(), "bigFont.fnt");
    label->setScale(.2f);
    label->setOpacity(140);
    label->setPosition({center, 7.f});

    // Narrow breaks stay unlabeled
    if (label->getScaledContentWidth() <= pause.right - pause.left + 4.f)
      m_plot->addChild(label);
  }

  // ! --- X labels: where every active part starts, and the end --- !
  struct DateLabel
  {
    float x;
    std::string text;
  };

  std::vector<DateLabel> candidates;

  for (std::size_t i = 0; i < m_knots.size(); ++i)
  {
    const bool afterBreak = std::any_of(
        m_breaks.begin(),
        m_breaks.end(),
        [&](Break const &pause)
        { return std::abs(pause.right - m_knots[i].x) < .01f; });

    if (i == 0 || afterBreak || i + 1 == m_knots.size())
      candidates.push_back({m_knots[i].x, formatShortDate(m_knots[i].time)});
  }

  if (!m_levelDone && candidates.size() > 1)
    candidates.back().text = "Now";

  std::vector<CCLabelBMFont *> placed;

  for (std::size_t i = 0; i < candidates.size(); ++i)
  {
    auto const &candidate = candidates[i];
    const bool isLast = i + 1 == candidates.size();

    if (candidate.text.empty() || (!placed.empty() && candidate.text == placed.back()->getString()))
      continue;

    auto label = CCLabelBMFont::create(candidate.text.c_str(), "bigFont.fnt");
    label->setScale(.25f);
    label->setOpacity(150);

    const float halfWidth = label->getScaledContentWidth() / 2;
    const float x = std::clamp(candidate.x, halfWidth, width - halfWidth);

    if (!placed.empty() && x - halfWidth < placed.back()->boundingBox().getMaxX() + 4.f)
    {
      // The end always shows, it takes the place of the label before it, but not of the first one
      if (!isLast || placed.size() < 2)
        continue;

      placed.back()->removeFromParent();
      placed.pop_back();
    }

    label->setPosition({x, -7.f});
    m_plot->addChild(label);
    placed.push_back(label);
  }
}

void StageTimelineChart::drawFlat(CCDrawNode *line, CCDrawNode *fill, float fromX, float toX, float y, bool dashed)
{
  const auto lineColor = ccc4FFromccc4B(LINE_COLOR);
  const auto fillColor = premultiplyAlpha(ccc4FFromccc4B(FILL_COLOR));
  const auto breakFillColor = premultiplyAlpha(ccc4FFromccc4B(FILL_BREAK_COLOR));

  float x = fromX;

  while (x < toX - .01f)
  {
    // The part up to the next break edge
    float end = toX;
    bool inBreak = false;

    for (auto const &pause : m_breaks)
    {
      if (x >= pause.left - .01f && x < pause.right - .01f)
      {
        end = std::min(end, pause.right);
        inBreak = true;
        break;
      }

      if (pause.left > x)
        end = std::min(end, pause.left);
    }

    if (inBreak || dashed)
      drawDashedLine(line, {x, y}, {end, y}, LINE_WIDTH * .8f, lineColor);
    else
      line->drawSegment({x, y}, {end, y}, LINE_WIDTH, lineColor);

    drawRect(fill, x, end, y, inBreak ? breakFillColor : fillColor);
    x = end;
  }
}

void StageTimelineChart::drawLine()
{
  auto fill = CCDrawNode::create();
  m_plot->addChild(fill, 1);

  auto line = CCDrawNode::create();
  m_plot->addChild(line, 2);

  const auto lineColor = ccc4FFromccc4B(LINE_COLOR);

  // ! --- Steps: flat until a stage is completed, then one up --- !
  CCPoint previous{getPointPosition(m_points.front()).x, 0.f};

  for (auto const &point : m_points)
  {
    const auto position = getPointPosition(point);

    drawFlat(line, fill, previous.x, position.x, previous.y, false);

    line->drawSegment({position.x, previous.y}, position, LINE_WIDTH, lineColor);
    previous = position;
  }

  // ! --- Still going: dashed until now --- !
  if (!m_levelDone)
    drawFlat(line, fill, previous.x, m_knots.back().x, previous.y, true);

  for (auto const &point : m_points)
    line->drawDot(getPointPosition(point), DOT_RADIUS, lineColor);
}

// Invisible buttons around the points, so touch screens can select them too.
// Every point owns the space up to the middle between it and its neighbours.
void StageTimelineChart::drawHitAreas()
{
  m_hitMenu->removeAllChildrenWithCleanup(true);

  const float height = m_plotSize.height + AXIS_BOTTOM;

  for (std::size_t i = 0; i < m_points.size(); ++i)
  {
    const float x = getPointPosition(m_points[i]).x;
    const float left = i == 0 ? 0.f : (getPointPosition(m_points[i - 1]).x + x) / 2;
    const float right = i + 1 == m_points.size() ? m_plotSize.width : (getPointPosition(m_points[i + 1]).x + x) / 2;
    const float width = std::max(1.f, right - left);

    auto area = CCNode::create();
    area->setContentSize({width, height});

    auto item = CCMenuItemSpriteExtra::create(
        area, this, menu_selector(StageTimelineChart::onPoint));
    item->setTag(static_cast<int>(i));
    item->m_scaleMultiplier = 1.f;
    item->setPosition({left + width / 2, height / 2 - AXIS_BOTTOM});

    m_hitMenu->addChild(item);
  }
}

// ! --- Selection --- !

void StageTimelineChart::select(int index, bool byHover)
{
  if (index < 0 || index >= static_cast<int>(m_points.size()))
    index = -1;

  m_selected = index;
  m_selectedByHover = byHover && index >= 0;

  m_selectedDot->clear();

  if (index >= 0)
  {
    const auto position = getPointPosition(m_points[index]);

    m_selectedDot->drawDot(position, SELECTED_DOT_RADIUS, {1.f, 1.f, 1.f, 1.f});
    m_selectedDot->drawDot(position, DOT_RADIUS, ccc4FFromccc4B(LINE_COLOR));
  }

  updateTooltip();
}

void StageTimelineChart::updateTooltip()
{
  if (m_selected < 0)
  {
    m_tooltip->setVisible(false);
    return;
  }

  auto const &point = m_points[m_selected];

  m_tooltipTitle->setText(fmt::format(
      "Stage {}   <small>{}</small>",
      point.stageIndex + 1,
      formatCompletedAt(point.time)));

  m_tooltipStats->setText(fmt::format(
      "{} <small>{}</small>   {}   {}/{} <small>stages</small>",
      formatCompactNumber(static_cast<float>(point.attempts)),
      point.attempts == 1 ? "attempt" : "attempts",
      formatTimePlayed(point.timePlayed),
      point.completed,
      m_totalStages));

  // ! --- Layout --- !
  const float padding = 5.f;
  const float titleHeight = 10.f;
  const float statsHeight = 9.f;
  const float hintHeight = 7.f;

  const float width = std::max({
                          m_tooltipTitle->getContentWidth(),
                          m_tooltipStats->getContentWidth(),
                          m_tooltipHint->getScaledContentWidth(),
                      }) +
                      padding * 2;
  const float height = titleHeight + statsHeight + hintHeight + padding * 2;

  m_tooltipBg->setSize({width, height});
  m_tooltipTitle->setPosition({padding, height - padding - titleHeight / 2});
  m_tooltipStats->setPosition({padding, height - padding - titleHeight - statsHeight / 2});
  m_tooltipHint->setPosition({padding, padding + hintHeight / 2});
  m_tooltip->setContentSize({width, height});

  // ! --- Position: up and right of the point, flipped when there is no room --- !
  const auto position = getPointPosition(point);
  const float offset = 6.f;

  float x = position.x + offset;
  float y = position.y + offset;

  if (x + width > m_plotSize.width)
    x = position.x - offset - width;

  if (y + height > m_plotSize.height)
    y = position.y - offset - height;

  // The tooltip may cover the Y labels, never leave the chart
  x = std::clamp(x, -AXIS_LEFT, m_plotSize.width - width);
  y = std::clamp(y, -AXIS_BOTTOM, m_plotSize.height + PLOT_TOP - height);

  m_tooltip->setPosition({x, y});
  m_tooltip->setVisible(true);
}

void StageTimelineChart::onPoint(CCObject *sender)
{
  auto *node = typeinfo_cast<CCNode *>(sender);

  if (!node)
    return;

  const int index = node->getTag();

  // The first tap selects, a tap on the selected point opens its stage.
  // With a mouse hover already selects, so one click opens.
  if (index == m_selected)
  {
    openSelected();
    return;
  }

  select(index, false);
}

void StageTimelineChart::moveSelection(int delta)
{
  const int count = static_cast<int>(m_points.size());

  if (count == 0)
    return;

  const int index = m_selected < 0
                        ? count - 1
                        : std::clamp(m_selected + delta, 0, count - 1);

  select(index, false);
}

void StageTimelineChart::openSelected()
{
  if (onOpenStage && m_selected >= 0 && m_selected < static_cast<int>(m_points.size()))
    onOpenStage(m_points[m_selected].stageIndex, {});
}

void StageTimelineChart::update(float dt)
{
  if (m_points.empty() || !nodeIsVisible(this))
    return;

  // A still mouse keeps the keyboard selection
  const auto mousePos = getMousePos();

  if (mousePos.equals(m_lastMousePos))
    return;

  m_lastMousePos = mousePos;

  const auto mouse = m_plot->convertToNodeSpace(mousePos);
  const bool inside =
      mouse.x >= 0.f && mouse.x < m_plotSize.width &&
      mouse.y >= -AXIS_BOTTOM && mouse.y <= m_plotSize.height;

  if (inside)
  {
    // The closest point along the time axis
    int closest = 0;
    float closestDistance = std::abs(getPointPosition(m_points[0]).x - mouse.x);

    for (std::size_t i = 1; i < m_points.size(); ++i)
    {
      const float distance = std::abs(getPointPosition(m_points[i]).x - mouse.x);

      if (distance < closestDistance)
      {
        closest = static_cast<int>(i);
        closestDistance = distance;
      }
    }

    if (closest != m_selected)
      select(closest, true);
  }
  else if (m_selectedByHover)
  {
    select(-1, false);
  }
}
