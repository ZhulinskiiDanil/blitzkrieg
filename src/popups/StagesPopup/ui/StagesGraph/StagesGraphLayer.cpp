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

  m_level = level;
  m_contentSize = contentSize;
  this->setContentSize(m_contentSize);

  auto profile = GlobalStore::get()->getProfileByLevel(m_level);
  auto &stages = profile.data.stages;
  auto metaInfo = getMetaInfoFromStages(stages);

  if (metaInfo.completed < 2)
  {
    auto bigFontLabel = CCLabelBMFont::create("First attach the profile!", "bigFont.fnt");
    bigFontLabel->setScale(.75f);
    bigFontLabel->setOpacity(255 * .6f);
    bigFontLabel->setPosition(contentSize / 2);

    this->addChild(bigFontLabel);
    return true;
  }

  // ===== CREATE GRAPH =====
  m_graph = Graph::create(contentSize);
  this->addChild(m_graph);

  std::vector<CCPoint> playtimeData;
  std::vector<CCPoint> attemptsData;

  int count = stages.size();

  float minY = std::numeric_limits<float>::infinity();
  float maxY = -std::numeric_limits<float>::infinity();

  for (int i = 0; i < count; i++)
  {
    float x = i + 1;

    float playtime = static_cast<float>(getStagePlaytime(&stages[i]));
    float attempts = static_cast<float>(getStageAttempts(&stages[i]));

    // собираем данные
    playtimeData.push_back({x, playtime});
    attemptsData.push_back({x, attempts});

    // общий min/max по всем линиям
    minY = std::min(minY, std::min(playtime, attempts));
    maxY = std::max(maxY, std::max(playtime, attempts));
  }

  int diffY = maxY - minY;

  // ===== GRID / LABELS =====
  m_graph->setGrid(1, diffY / 10, {60, 60, 60, 255});
  m_graph->setLabels(1, diffY / 10, {200, 200, 200, 255});

  // ===== LINES =====
  m_graph->addLine(playtimeData, ccc3(0, 255, 0));     // зелёная (время)
  m_graph->addLine(attemptsData, ccc3(255, 100, 100)); // красная (попытки)

  m_graph->redraw();

  return true;
}