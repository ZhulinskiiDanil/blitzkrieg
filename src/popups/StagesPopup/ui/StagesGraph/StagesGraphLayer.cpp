#include "StagesGraphLayer.hpp"

StagesGraphLayer *StagesGraphLayer::create(GJGameLevel *level, const CCSize &contentSize)
{
  auto ret = new StagesGraphLayer();
  if (ret && ret->init(level, contentSize))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool StagesGraphLayer::init(GJGameLevel *level, const CCSize &contentSize)
{
  if (!CCLayer::init())
    return false;

  Padding padding{45.f, 45.f, 30.f, 10.f}; // top, bottom, left, right

  m_level = level;
  m_size = contentSize;
  this->setContentSize(m_size);

  const auto innerSize = CCSize(
      m_size.width - padding.left - padding.right,
      m_size.height - padding.top - padding.bottom);

  auto profile = GlobalStore::get()->getProfileByLevel(m_level);
  auto &stages = profile.data.stages;
  auto metaInfo = getMetaInfoFromStages(stages);

  CCLabelBMFont *bigFontLabel;

  if (profile.id.empty())
    bigFontLabel = CCLabelBMFont::create("Attach your profile first", "bigFont.fnt");
  else if (metaInfo.completed < 3)
    bigFontLabel = CCLabelBMFont::create("Complete at least 3 stages", "bigFont.fnt");

  if (bigFontLabel)
  {
    bigFontLabel->setScale(.75f);
    bigFontLabel->setOpacity(255 * .6f);
    bigFontLabel->setPosition(m_size / 2);
    this->addChild(bigFontLabel);
    return true;
  }

  // ! --- LABELS --- !
  auto labelsList = CCLayer::create();
  labelsList->setAnchorPoint({0, 0.5f});
  labelsList->setPosition({30, m_size.height - padding.top / 2 - 5});
  labelsList->setLayout(RowLayout::create()
                            ->setGap(10)
                            ->setAutoScale(false)
                            ->setAutoGrowAxis(true));

  for (auto i = 0; i < 2; i++)
  {
    bool isPlaytime = i == 0;
    auto text = isPlaytime ? "Playtime (seconds)" : "Attempts";
    auto color = isPlaytime
                     ? ccc4FFromccc4B({0, 255, 0, 255})
                     : ccc4FFromccc4B({255, 100, 100, 255});
    auto block = CCLayer::create();
    block->setLayout(RowLayout::create()
                         ->setGap(5)
                         ->setAutoScale(false)
                         ->setAutoGrowAxis(true)
                         ->setAxisAlignment(AxisAlignment::Start));
    block->setContentSize({m_size.width / 4, 20});

    auto rectNode = RectNode::create({14, 14}, color, 4);
    block->addChild(rectNode);

    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.4f);
    block->addChild(label);

    labelsList->addChild(block);
    block->updateLayout();
  }

  this->addChild(labelsList);
  labelsList->updateLayout();

  // ! --- CREATE GRAPH --- !
  m_graph = Graph::create(innerSize);
  m_graph->setPosition({padding.left, padding.bottom});
  this->addChild(m_graph);

  std::vector<GraphDot> playtimeData;
  std::vector<GraphDot> attemptsData;

  int count = stages.size();

  float minY = std::numeric_limits<float>::infinity();
  float maxY = -std::numeric_limits<float>::infinity();

  for (int i = 0; i < count; i++)
  {
    float x = i + 1;

    float playtime = static_cast<float>(getStagePlaytime(&stages[i]));
    float attempts = static_cast<float>(getStageAttempts(&stages[i]));

    playtimeData.push_back({x, playtime});
    attemptsData.push_back({x, attempts});

    minY = std::min(minY, std::min(playtime, attempts));
    maxY = std::max(maxY, std::max(playtime, attempts));
  }

  int diffY = maxY - minY;

  // ! --- GRID --- !
  m_graph->setGrid(1, diffY / 10, {60, 60, 60, 255});
  m_graph->setLabels(1, diffY / 10, {200, 200, 200, 255});

  // ! --- LINES --- !
  auto playtimeLineUUID = m_graph->addLine(playtimeData, ccc3(0, 255, 0), ccc3(130, 130, 130));
  m_graph->setFormatterByLineUUID(playtimeLineUUID, [this](float x, float y, float val)
                                  { return formatTimePlayed(val); });

  auto attemptsLineUUID = m_graph->addLine(attemptsData, ccc3(255, 100, 100), ccc3(130, 130, 130));
  m_graph->setFormatterByLineUUID(attemptsLineUUID, [this](float x, float y, float val)
                                  { return fmt::format("{} <small>Attempts</small>", val); });

  m_graph->redraw();

  return true;
}