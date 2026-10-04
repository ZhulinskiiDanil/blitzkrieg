#include "StageBarChart.hpp"

#include <algorithm>
#include <cmath>

#include "../../../../utils/formatTimePlayed.hpp"

namespace
{
  const ccColor3B COMPLETED_COLOR{99, 224, 110};
  const ccColor3B CURRENT_COLOR{255, 220, 90};
  const ccColor3B UPCOMING_COLOR{110, 110, 110};
  const ccColor4B GRID_COLOR{60, 60, 60, 255};
  const ccColor4B BASELINE_COLOR{90, 90, 90, 255};
  const ccColor4B BEST_LINE_COLOR{100, 215, 255, 220};

  // UILabel has no height, it is centered on its y
  UILabel *createLabel(std::string const &text, float scale, float anchorX)
  {
    auto label = UILabel::create(text, "bigFont.fnt", scale);
    label->ignoreAnchorPointForPosition(false);
    label->setAnchorPoint({anchorX, .5f});
    return label;
  }

  ccColor3B getStatusColor(StageGraphStatus status)
  {
    switch (status)
    {
    case StageGraphStatus::Completed:
      return COMPLETED_COLOR;
    case StageGraphStatus::Current:
      return CURRENT_COLOR;
    default:
      return UPCOMING_COLOR;
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
    return metric == StageGraphMetric::Attempts
               ? formatCompactNumber(value)
               : formatTimePlayed(value);
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

  m_bestNode = CCNode::create();
  m_plot->addChild(m_bestNode, 3);

  m_hitMenu = CCMenu::create();
  m_hitMenu->setPosition({0.f, 0.f});
  m_plot->addChild(m_hitMenu, 4);

  // ! --- Tooltip --- !
  m_tooltip = CCNode::create();
  m_tooltip->setVisible(false);
  m_plot->addChild(m_tooltip, 5);

  m_tooltipBg = RectNode::create({1.f, 1.f}, premultiplyAlpha(ccc4FFromccc4B({10, 10, 10, 235})), 4);
  m_tooltip->addChild(m_tooltipBg);

  m_tooltipLabel = createLabel("", .3f, 0.f);
  m_tooltip->addChild(m_tooltipLabel);

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

// ! --- Data --- !

void StageBarChart::setData(std::vector<StageGraphColumn> columns, StageGraphMetric metric)
{
  m_columns = std::move(columns);
  m_metric = metric;
  m_selected = -1;
  m_selectedByHover = false;

  rebuild(true);
}

void StageBarChart::setMetric(StageGraphMetric metric)
{
  if (metric == m_metric)
    return;

  m_metric = metric;
  rebuild(true);
}

void StageBarChart::setBestFromZero(std::optional<float> x, float percent)
{
  m_bestX = x;
  m_bestPercent = percent;
  drawBestLine();
}

// ! --- Drawing --- !

void StageBarChart::rebuild(bool animate)
{
  // ! Axis max: 0 to a round value above the highest bar
  float maxValue = 0.f;

  for (auto const &column : m_columns)
    maxValue = std::max(maxValue, column.getValue(m_metric));

  const float raw = maxValue / GRID_LINES;
  const float step = m_metric == StageGraphMetric::Attempts
                         ? getNiceAxisStep(raw)
                         : getNiceTimeStep(raw);

  m_axisStep = step;
  m_axisMax = std::max(step, std::ceil(maxValue / step) * step);

  drawAxis();
  drawBars(animate);
  drawBestLine();
  drawHitAreas();
  select(m_selected, m_selectedByHover);
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

void StageBarChart::drawBars(bool animate)
{
  m_barsNode->removeAllChildrenWithCleanup(true);
  m_bars.clear();

  const float slot = getSlotWidth();
  const float barWidth = std::max(1.f, std::min(MAX_BAR_WIDTH, slot * BAR_WIDTH_RATIO));

  for (std::size_t i = 0; i < m_columns.size(); ++i)
  {
    auto const &column = m_columns[i];
    const float barHeight = getBarHeight(column);

    auto color = ccc4FFromccc3B(getStatusColor(column.status));
    if (column.status == StageGraphStatus::Upcoming)
      color.a = .6f;

    const float radius = std::min({2.f, barWidth / 2, barHeight / 2});

    auto bar = RectNode::create({barWidth, std::max(barHeight, .01f)}, premultiplyAlpha(color), radius);
    bar->setAnchorPoint({0.f, 0.f});
    bar->setPosition({getColumnCenterX(static_cast<float>(i)) - barWidth / 2, 0.f});
    bar->setVisible(barHeight > 0.f);
    m_barsNode->addChild(bar);
    m_bars.push_back(bar);

    if (animate)
    {
      // Bars grow from the baseline one after another
      bar->setScaleY(0.f);
      bar->runAction(CCSequence::createWithTwoActions(
          CCDelayTime::create(std::min(.15f, i * .012f)),
          CCEaseOut::create(CCScaleTo::create(.25f, 1.f, 1.f), 2.f)));
    }
  }
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

void StageBarChart::select(int index, bool byHover)
{
  if (index < 0 || index >= static_cast<int>(m_columns.size()))
    index = -1;

  m_selected = index;
  m_selectedByHover = byHover && index >= 0;

  m_highlight->setVisible(index >= 0);

  if (index >= 0)
  {
    const float slot = getSlotWidth();

    m_highlight->setSize({slot, m_plotSize.height});
    m_highlight->setPosition({slot * index, 0.f});
  }

  updateTooltip();
}

void StageBarChart::updateTooltip()
{
  if (m_selected < 0)
  {
    m_tooltip->setVisible(false);
    return;
  }

  auto const &column = m_columns[m_selected];

  m_tooltipLabel->setText(fmt::format(
      "Stage {}   {} <small>{}</small>   {}   {}/{} <small>runs</small>",
      column.index + 1,
      formatCompactNumber(static_cast<float>(column.attempts)),
      column.attempts == 1 ? "attempt" : "attempts",
      formatTimePlayed(column.timePlayed),
      column.completedRuns,
      column.totalRuns));

  // ! --- Layout --- !
  const float padding = 5.f;
  const float lineHeight = 10.f;
  const float hintHeight = 7.f;

  const float width = std::max(
                          m_tooltipLabel->getContentWidth(),
                          m_tooltipHint->getScaledContentWidth()) +
                      padding * 2;
  const float height = lineHeight + hintHeight + padding * 2;

  m_tooltipBg->setSize({width, height});
  m_tooltipLabel->setPosition({padding, height - padding - lineHeight / 2});
  m_tooltipHint->setPosition({padding, padding + hintHeight / 2});
  m_tooltip->setContentSize({width, height});

  // ! --- Position: above the bar, or beside the column when there is no room --- !
  const float slot = getSlotWidth();
  const float centerX = getColumnCenterX(static_cast<float>(m_selected));
  const float barTop = m_selected < static_cast<int>(m_bars.size())
                           ? getBarHeight(column)
                           : 0.f;

  float x = centerX - width / 2;
  float y = barTop + 4.f;

  if (y + height > m_plotSize.height)
  {
    x = centerX + slot / 2 + 3.f;

    if (x + width > m_plotSize.width)
      x = centerX - slot / 2 - 3.f - width;

    y = m_plotSize.height - height;
  }

  // The tooltip may cover the Y labels, never leave the chart
  x = std::clamp(x, -AXIS_LEFT, m_plotSize.width - width);
  y = std::clamp(y, -AXIS_BOTTOM, m_plotSize.height + PLOT_TOP - height);

  m_tooltip->setPosition({x, y});
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

  select(index, false);
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

    if (index != m_selected)
      select(index, true);
  }
  else if (m_selectedByHover)
  {
    select(-1, false);
  }
}
