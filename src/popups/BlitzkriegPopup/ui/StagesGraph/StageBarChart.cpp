#include "StageBarChart.hpp"

#include <algorithm>
#include <cmath>

#include "../../../../utils/formatCompletedAt.hpp"
#include "../../../../utils/formatTimePlayed.hpp"

namespace
{
  const ccColor3B COMPLETED_COLOR{99, 224, 110};
  const ccColor3B CURRENT_COLOR{255, 220, 90};
  const ccColor3B UPCOMING_COLOR{110, 110, 110};
  const ccColor4B GRID_COLOR{60, 60, 60, 255};
  const ccColor4B BASELINE_COLOR{90, 90, 90, 255};
  const ccColor4B BEST_LINE_COLOR{100, 215, 255, 220};
  const ccColor4B AVERAGE_LINE_COLOR{255, 255, 255, 110};
  const ccColor4B PLACEHOLDER_COLOR{110, 110, 110, 64};

  // Dark and see-through, a hint of where one run ends
  const ccColor4B DIVIDER_COLOR{0, 0, 0, 90};
  constexpr float DIVIDER_WIDTH = .3f;

  // UILabel has no height, it is centered on its y
  UILabel *createLabel(std::string const &text, float scale, float anchorX)
  {
    auto label = UILabel::create(text, "bigFont.fnt", scale);
    label->ignoreAnchorPointForPosition(false);
    label->setAnchorPoint({anchorX, .5f});
    return label;
  }

  ccColor4F getStatusColor(StageGraphStatus status)
  {
    switch (status)
    {
    case StageGraphStatus::Completed:
      return ccc4FFromccc3B(COMPLETED_COLOR);
    case StageGraphStatus::Current:
      return ccc4FFromccc3B(CURRENT_COLOR);
    default:
      // Upcoming stages stay in the background
      return premultiplyAlpha({UPCOMING_COLOR.r / 255.f, UPCOMING_COLOR.g / 255.f, UPCOMING_COLOR.b / 255.f, .6f});
    }
  }

  // Steps that read well as time, in seconds
  float getNiceTimeStep(float raw)
  {
    static const float steps[] = {
        10.f, 30.f, 60.f, 120.f, 300.f, 600.f, 900.f, 1800.f,
        3600.f, 7200.f, 10800.f, 18000.f, 36000.f, 86400.f};

    for (float step : steps)
    {
      if (step >= raw)
        return step;
    }

    return std::ceil(raw / 86400.f) * 86400.f;
  }

  std::string formatValue(float value, StageGraphMetric metric)
  {
    return metric == StageGraphMetric::Playtime
               ? formatTimePlayed(value)
               : formatCompactNumber(value);
  }

  // 12.5%, 40%
  std::string formatPercent(float value)
  {
    std::string number = fmt::format("{:.2f}", value);

    while (number.back() == '0')
      number.pop_back();

    if (number.back() == '.')
      number.pop_back();

    return number + "%";
  }

  float easeOutCubic(float t)
  {
    const float inv = 1.f - t;
    return 1.f - inv * inv * inv;
  }
}

StageBarChart *StageBarChart::create(const CCSize &size)
{
  auto ret = new StageBarChart();
  if (ret && ret->init(size))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StageBarChart::init(const CCSize &size)
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

  // ! --- Layers, bottom to top --- !
  m_highlight = RectNode::create({1.f, 1.f}, premultiplyAlpha(ccc4FFromccc4B({255, 255, 255, 10})), 3);
  m_highlight->setVisible(false);
  m_plot->addChild(m_highlight, 0);

  m_axisNode = CCNode::create();
  m_plot->addChild(m_axisNode, 1);

  m_barsNode = CCNode::create();
  m_plot->addChild(m_barsNode, 2);

  m_segmentHighlight = RectNode::create({1.f, 1.f}, premultiplyAlpha(ccc4FFromccc4B({255, 255, 255, 56})), 2);
  m_segmentHighlight->setVisible(false);
  m_plot->addChild(m_segmentHighlight, 3);

  m_averageNode = CCNode::create();
  m_plot->addChild(m_averageNode, 4);

  m_bestNode = CCNode::create();
  m_plot->addChild(m_bestNode, 5);

  m_hitMenu = CCMenu::create();
  m_hitMenu->setPosition({0.f, 0.f});
  m_plot->addChild(m_hitMenu, 6);

  // ! --- Tooltip --- !
  m_tooltip = CCNode::create();
  m_tooltip->setVisible(false);
  m_plot->addChild(m_tooltip, 7);

  m_tooltipBg = RectNode::create({1.f, 1.f}, premultiplyAlpha(ccc4FFromccc4B({10, 10, 10, 235})), 4);
  m_tooltip->addChild(m_tooltipBg);

  for (int i = 0; i < 3; ++i)
  {
    // The first line is the stage, the others are a bit smaller
    auto line = createLabel("", i == 0 ? .3f : .26f, 0.f);
    m_tooltip->addChild(line);
    m_tooltipLines.push_back(line);
  }

  m_tooltipHint = CCLabelBMFont::create("Click to open in Stage Browser", "bigFont.fnt");
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

float StageBarChart::getSlotWidth() const
{
  return m_columns.empty() ? m_plotSize.width : m_plotSize.width / m_columns.size();
}

float StageBarChart::getColumnCenterX(float x) const
{
  return getSlotWidth() * (x + .5f);
}

float StageBarChart::getBarHeight(StageGraphColumn const &column) const
{
  const float value = column.getValue(m_metric);

  if (value <= 0.f || m_axisMax <= 0.f)
    return 0.f;

  // Tiny values still show a sliver
  return std::max(1.5f, m_plotSize.height * value / m_axisMax);
}

std::vector<float> StageBarChart::getSegmentHeights(StageGraphColumn const &column) const
{
  std::vector<float> heights(column.runs.size(), 0.f);
  const float barHeight = getBarHeight(column);

  if (barHeight <= 0.f || heights.empty())
    return heights;

  float sum = 0.f;

  for (auto const &run : column.runs)
    sum += run.getValue(m_metric);

  if (sum <= 0.f)
  {
    heights.back() = barHeight;
    return heights;
  }

  for (std::size_t i = 0; i < heights.size(); ++i)
    heights[i] = barHeight * column.runs[i].getValue(m_metric) / sum;

  // ! Thin runs go into the closest thick run below, or above for the bottom ones
  for (std::size_t i = 0; i < heights.size(); ++i)
  {
    if (heights[i] <= 0.f || heights[i] >= MIN_SEGMENT_HEIGHT)
      continue;

    int target = -1;

    for (int j = static_cast<int>(i) - 1; j >= 0 && target < 0; --j)
    {
      if (heights[j] >= MIN_SEGMENT_HEIGHT)
        target = j;
    }

    for (std::size_t j = i + 1; j < heights.size() && target < 0; ++j)
    {
      if (heights[j] > 0.f)
        target = static_cast<int>(j);
    }

    // The only run of the bar stays as it is
    if (target < 0)
      continue;

    heights[target] += heights[i];
    heights[i] = 0.f;
  }

  return heights;
}

int StageBarChart::getRunAt(int column, float y) const
{
  if (column < 0 || column >= static_cast<int>(m_bars.size()))
    return -1;

  auto const &segments = m_bars[column].segments;

  for (std::size_t i = 0; i < segments.size(); ++i)
  {
    auto const &segment = segments[i];

    // The gap above a run belongs to it
    if (segment.toHeight > 0.f &&
        y >= segment.toY &&
        y < segment.toY + segment.toHeight)
      return static_cast<int>(i);
  }

  return -1;
}

// ! --- Data --- !

void StageBarChart::setData(std::vector<StageGraphColumn> columns, StageGraphMetric metric)
{
  m_columns = std::move(columns);
  m_metric = metric;
  m_selected = -1;
  m_selectedRun = -1;
  m_selectedByHover = false;

  rebuild(true);
}

void StageBarChart::setMetric(StageGraphMetric metric)
{
  if (metric == m_metric)
    return;

  m_metric = metric;
  rebuild(false);
}

void StageBarChart::setBestFromZero(std::optional<float> x, float percent)
{
  m_bestX = x;
  m_bestPercent = percent;
  drawBestLine();
}

// ! --- Drawing --- !

// `recreate` builds the bars again and grows them from the baseline,
// otherwise the bars move from their current height to the new one
void StageBarChart::rebuild(bool recreate)
{
  // ! Axis max: 0 to a round value above the highest bar
  float maxValue = 0.f;

  for (auto const &column : m_columns)
    maxValue = std::max(maxValue, column.getValue(m_metric));

  const float raw = maxValue / GRID_LINES;
  const float step = m_metric == StageGraphMetric::Playtime
                         ? getNiceTimeStep(raw)
                         : getNiceAxisStep(raw);

  m_axisStep = step;
  m_axisMax = std::max(step, std::ceil(maxValue / step) * step);

  drawAxis();

  if (recreate)
  {
    createBars();
    drawHitAreas();
  }

  retargetBars(recreate);
  drawAverageLine();
  drawBestLine();
  select(m_selected, m_selectedRun, m_selectedByHover);
}

void StageBarChart::drawAxis()
{
  m_axisNode->removeAllChildrenWithCleanup(true);

  const float width = m_plotSize.width;
  const float height = m_plotSize.height;

  // ! --- Horizontal grid and Y labels --- !
  auto grid = CCDrawNode::create();
  m_axisNode->addChild(grid);

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

    auto label = createLabel(formatValue(value, m_metric), .25f, 1.f);
    label->setPosition({-5.f, y});
    m_axisNode->addChild(label);
  }

  // ! --- X labels: stage numbers, thinned out when columns are narrow --- !
  const float slot = getSlotWidth();
  const int every = std::max(1, static_cast<int>(std::ceil(14.f / slot)));

  for (std::size_t i = 0; i < m_columns.size(); ++i)
  {
    const bool isLast = i + 1 == m_columns.size();

    if (i % every != 0 && !isLast)
      continue;

    auto label = CCLabelBMFont::create(std::to_string(i + 1).c_str(), "bigFont.fnt");
    label->setScale(.25f);
    label->setOpacity(150);
    label->setPosition({getColumnCenterX(static_cast<float>(i)), -7.f});
    m_axisNode->addChild(label);
  }
}

void StageBarChart::createBars()
{
  this->unschedule(schedule_selector(StageBarChart::onTween));
  m_barsNode->removeAllChildrenWithCleanup(true);
  m_bars.clear();

  const float slot = getSlotWidth();
  m_barWidth = std::max(1.f, std::min(MAX_BAR_WIDTH, slot * BAR_WIDTH_RATIO));

  for (std::size_t i = 0; i < m_columns.size(); ++i)
  {
    auto const &column = m_columns[i];
    const float x = getColumnCenterX(static_cast<float>(i)) - m_barWidth / 2;

    BarColumn bar;

    // A stage without attempts still shows where it is
    bar.placeholder = RectNode::create(
        {m_barWidth, PLACEHOLDER_HEIGHT},
        premultiplyAlpha(ccc4FFromccc4B(PLACEHOLDER_COLOR)),
        std::min(1.5f, m_barWidth / 2));
    bar.placeholder->setPosition({x, 0.f});
    m_barsNode->addChild(bar.placeholder);

    bar.x = x;
    bar.bar = RectNode::create({m_barWidth, 1.f}, getStatusColor(column.status), 0.f);
    bar.bar->setPosition({x, 0.f});
    bar.bar->setVisible(false);
    m_barsNode->addChild(bar.bar);

    bar.dividers = CCDrawNode::create();
    m_barsNode->addChild(bar.dividers);

    bar.segments.resize(column.runs.size());

    m_bars.push_back(std::move(bar));
  }
}

void StageBarChart::retargetBars(bool fromZero)
{
  float lastDelay = 0.f;

  for (std::size_t i = 0; i < m_bars.size(); ++i)
  {
    auto &bar = m_bars[i];
    const auto heights = getSegmentHeights(m_columns[i]);

    bool empty = true;
    float y = 0.f;

    for (std::size_t k = 0; k < bar.segments.size(); ++k)
    {
      auto &segment = bar.segments[k];
      const float height = heights[k];

      segment.fromY = fromZero ? 0.f : segment.y;
      segment.fromHeight = fromZero ? 0.f : segment.height;
      segment.toY = y;
      segment.toHeight = height;

      empty = empty && height <= 0.f;
      y += height;
    }

    bar.placeholder->setVisible(empty);

    // Bars grow from the baseline one after another
    bar.delay = fromZero ? std::min(.15f, i * .012f) : 0.f;
    lastDelay = std::max(lastDelay, bar.delay);
  }

  m_tweenTime = 0.f;
  m_tweenEnd = lastDelay + TWEEN_DURATION;

  applyBars(0.f);

  this->unschedule(schedule_selector(StageBarChart::onTween));
  this->schedule(schedule_selector(StageBarChart::onTween));
}

void StageBarChart::applyBars(float time)
{
  for (auto &bar : m_bars)
  {
    const float progress = easeOutCubic(std::clamp((time - bar.delay) / TWEEN_DURATION, 0.f, 1.f));
    float total = 0.f;

    for (auto &segment : bar.segments)
    {
      segment.y = segment.fromY + (segment.toY - segment.fromY) * progress;
      segment.height = segment.fromHeight + (segment.toHeight - segment.fromHeight) * progress;
      total = std::max(total, segment.y + segment.height);
    }

    const bool visible = total > .05f;
    bar.bar->setVisible(visible);
    bar.dividers->clear();

    if (!visible)
      continue;

    bar.bar->setRadius(std::min({2.f, m_barWidth / 2, total / 2}));
    bar.bar->setSize({m_barWidth, total});

    // ! A divider at the bottom of every run but the first one
    for (auto const &segment : bar.segments)
    {
      if (segment.height <= .05f || segment.y <= .05f)
        continue;

      bar.dividers->drawSegment(
          {bar.x, segment.y},
          {bar.x + m_barWidth, segment.y},
          DIVIDER_WIDTH,
          premultiplyAlpha(ccc4FFromccc4B(DIVIDER_COLOR)));
    }
  }
}

void StageBarChart::onTween(float dt)
{
  m_tweenTime += dt;

  if (m_tweenTime >= m_tweenEnd)
  {
    applyBars(m_tweenEnd);
    this->unschedule(schedule_selector(StageBarChart::onTween));
    updateHighlight();
    return;
  }

  applyBars(m_tweenTime);
}

void StageBarChart::drawAverageLine()
{
  m_averageNode->removeAllChildrenWithCleanup(true);

  float sum = 0.f;
  int count = 0;

  for (auto const &column : m_columns)
  {
    const float value = column.getValue(m_metric);

    if (value <= 0.f)
      continue;

    sum += value;
    count++;
  }

  // One bar is its own average
  if (count < 2 || m_axisMax <= 0.f)
    return;

  const float average = sum / count;
  const float y = m_plotSize.height * average / m_axisMax;
  const auto color = premultiplyAlpha(ccc4FFromccc4B(AVERAGE_LINE_COLOR));

  // ! --- Dashed horizontal line --- !
  auto line = CCDrawNode::create();
  const float dash = 3.f;
  const float gap = 3.f;

  for (float x = 0.f; x < m_plotSize.width; x += dash + gap)
  {
    line->drawSegment(
        {x, y},
        {std::min(x + dash, m_plotSize.width), y},
        .4f,
        color);
  }

  m_averageNode->addChild(line);

  // ! --- Label at the right end, above the line --- !
  auto label = createLabel(fmt::format("<small>avg</small> {}", formatValue(average, m_metric)), .22f, 1.f);
  label->setPosition({m_plotSize.width, std::min(y + 5.f, m_plotSize.height + PLOT_TOP / 2)});
  m_averageNode->addChild(label);
}

void StageBarChart::drawBestLine()
{
  m_bestNode->removeAllChildrenWithCleanup(true);

  if (!m_bestX || m_columns.empty())
    return;

  const float x = std::clamp(getColumnCenterX(*m_bestX), 0.f, m_plotSize.width);

  // ! --- Dashed vertical line --- !
  auto line = CCDrawNode::create();
  const float dash = 3.f;
  const float gap = 3.f;

  for (float y = 0.f; y < m_plotSize.height; y += dash + gap)
  {
    line->drawSegment(
        {x, y},
        {x, std::min(y + dash, m_plotSize.height)},
        .5f,
        premultiplyAlpha(ccc4FFromccc4B(BEST_LINE_COLOR)));
  }

  m_bestNode->addChild(line);

  // ! --- Label above the plot, kept inside the chart --- !
  auto label = CCLabelBMFont::create(
      fmt::format("Best from 0%: {}%", static_cast<int>(m_bestPercent)).c_str(),
      "bigFont.fnt");
  label->setScale(.25f);
  label->setColor({BEST_LINE_COLOR.r, BEST_LINE_COLOR.g, BEST_LINE_COLOR.b});

  const float halfWidth = label->getScaledContentWidth() / 2;
  const float labelX = std::clamp(
      x,
      -AXIS_LEFT + halfWidth,
      m_plotSize.width - halfWidth);

  label->setPosition({labelX, m_plotSize.height + PLOT_TOP / 2});
  m_bestNode->addChild(label);
}

// Invisible buttons over the columns, so touch screens can select them too
void StageBarChart::drawHitAreas()
{
  m_hitMenu->removeAllChildrenWithCleanup(true);

  const float slot = getSlotWidth();
  const CCSize hitSize{slot, m_plotSize.height + AXIS_BOTTOM};

  for (std::size_t i = 0; i < m_columns.size(); ++i)
  {
    auto area = CCNode::create();
    area->setContentSize(hitSize);

    auto item = CCMenuItemSpriteExtra::create(
        area, this, menu_selector(StageBarChart::onColumn));
    item->setTag(static_cast<int>(i));
    item->m_scaleMultiplier = 1.f;
    item->setPosition({
        getColumnCenterX(static_cast<float>(i)),
        hitSize.height / 2 - AXIS_BOTTOM,
    });

    m_hitMenu->addChild(item);
  }
}

// ! --- Selection --- !

void StageBarChart::select(int index, int run, bool byHover)
{
  if (index < 0 || index >= static_cast<int>(m_columns.size()))
    index = -1;

  if (index < 0 || run < 0 || run >= static_cast<int>(m_columns[index].runs.size()))
    run = -1;

  m_selected = index;
  m_selectedRun = run;
  m_selectedByHover = byHover && index >= 0;

  updateHighlight();
  updateTooltip();
}

void StageBarChart::updateHighlight()
{
  m_highlight->setVisible(m_selected >= 0);
  m_segmentHighlight->setVisible(false);

  if (m_selected < 0)
    return;

  const float slot = getSlotWidth();

  m_highlight->setSize({slot, m_plotSize.height});
  m_highlight->setPosition({slot * m_selected, 0.f});

  if (m_selectedRun < 0 || m_selected >= static_cast<int>(m_bars.size()))
    return;

  auto const &segment = m_bars[m_selected].segments[m_selectedRun];

  if (segment.toHeight <= 0.f)
    return;

  m_segmentHighlight->setRadius(std::min({1.f, m_barWidth / 2, segment.toHeight / 2}));
  m_segmentHighlight->setSize({m_barWidth, segment.toHeight});
  m_segmentHighlight->setPosition({
      getColumnCenterX(static_cast<float>(m_selected)) - m_barWidth / 2,
      segment.toY,
  });
  m_segmentHighlight->setVisible(true);
}

void StageBarChart::updateTooltip()
{
  if (m_selected < 0)
  {
    m_tooltip->setVisible(false);
    return;
  }

  auto const &column = m_columns[m_selected];

  // ! --- Text --- !
  std::vector<std::string> lines;

  lines.push_back(fmt::format(
      "Stage {}   {} <small>{}</small>   {}   {}/{} <small>runs</small>",
      column.index + 1,
      formatCompactNumber(static_cast<float>(column.attempts)),
      column.attempts == 1 ? "attempt" : "attempts",
      formatTimePlayed(column.timePlayed),
      column.completedRuns,
      column.totalRuns));

  if (m_selectedRun >= 0)
  {
    auto const &run = column.runs[m_selectedRun];

    lines.push_back(fmt::format(
        "{} - {}   {} <small>{}</small>   {}",
        formatPercent(run.from),
        formatPercent(run.to),
        formatCompactNumber(static_cast<float>(run.attempts)),
        run.attempts == 1 ? "attempt" : "attempts",
        formatTimePlayed(run.timePlayed)));

    std::string details;

    if (run.range && run.range->bestRunFrom >= 0.f && run.range->bestRunTo > 0.f)
    {
      details += fmt::format(
          "<small>Best</small> {} - {}",
          formatPercent(run.range->bestRunFrom),
          formatPercent(run.range->bestRunTo));
    }

    if (auto date = formatCompletedAt(run.completedAt); !date.empty())
      details += fmt::format("{}<small>Completed</small> {}", details.empty() ? "" : "   ", date);

    if (!details.empty())
      lines.push_back(details);
  }
  else if (auto date = formatCompletedAt(column.completedAt); !date.empty())
  {
    lines.push_back(fmt::format("<small>Completed</small> {}", date));
  }

  // ! --- Layout, top to bottom --- !
  const float padding = 5.f;
  const float firstLineHeight = 10.f;
  const float lineHeight = 9.f;
  const float hintHeight = 7.f;

  float width = m_tooltipHint->getScaledContentWidth();
  float height = padding * 2 + hintHeight;

  for (std::size_t i = 0; i < m_tooltipLines.size(); ++i)
  {
    auto *label = m_tooltipLines[i];
    const bool visible = i < lines.size();

    label->setVisible(visible);

    if (!visible)
      continue;

    label->setText(lines[i]);
    width = std::max(width, label->getContentWidth());
    height += i == 0 ? firstLineHeight : lineHeight;
  }

  width += padding * 2;

  float y = height - padding;

  for (std::size_t i = 0; i < lines.size(); ++i)
  {
    const float line = i == 0 ? firstLineHeight : lineHeight;
    m_tooltipLines[i]->setPosition({padding, y - line / 2});
    y -= line;
  }

  m_tooltipHint->setPosition({padding, padding + hintHeight / 2});
  m_tooltipBg->setSize({width, height});
  m_tooltip->setContentSize({width, height});

  // ! --- Position: above the bar, or beside the column when there is no room --- !
  const float slot = getSlotWidth();
  const float centerX = getColumnCenterX(static_cast<float>(m_selected));
  const float barTop = getBarHeight(column);

  float x = centerX - width / 2;
  float top = barTop + 4.f;

  if (top + height > m_plotSize.height)
  {
    x = centerX + slot / 2 + 3.f;

    if (x + width > m_plotSize.width)
      x = centerX - slot / 2 - 3.f - width;

    top = m_plotSize.height - height;
  }

  // The tooltip may cover the Y labels, never leave the chart
  x = std::clamp(x, -AXIS_LEFT, m_plotSize.width - width);
  top = std::clamp(top, -AXIS_BOTTOM, m_plotSize.height + PLOT_TOP - height);

  m_tooltip->setPosition({x, top});
  m_tooltip->setVisible(true);
}

void StageBarChart::onColumn(CCObject *sender)
{
  auto *node = typeinfo_cast<CCNode *>(sender);

  if (!node)
    return;

  const int index = node->getTag();

  // The first tap selects, a tap on the selected column opens it.
  // With a mouse hover already selects, so one click opens.
  if (index == m_selected)
  {
    if (onOpenStage)
      onOpenStage(m_columns[index].index);

    return;
  }

  select(index, -1, false);
}

void StageBarChart::update(float dt)
{
  if (m_columns.empty() || !nodeIsVisible(this))
    return;

  const auto mouse = m_plot->convertToNodeSpace(getMousePos());
  const bool inside =
      mouse.x >= 0.f && mouse.x < m_plotSize.width &&
      mouse.y >= -AXIS_BOTTOM && mouse.y <= m_plotSize.height;

  if (inside)
  {
    const int index = static_cast<int>(mouse.x / getSlotWidth());
    const int run = getRunAt(index, mouse.y);

    if (index != m_selected || run != m_selectedRun)
      select(index, run, true);
  }
  else if (m_selectedByHover)
  {
    select(-1, -1, false);
  }
}
