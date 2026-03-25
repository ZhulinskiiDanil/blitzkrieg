#pragma once
#include <Geode/Geode.hpp>

#include "../../../../ui/types/index.hpp"
#include "../../../../ui/Graph.hpp"
#include "../../../../ui/RectNode.hpp"
#include "../../../../store/GlobalStore.hpp"
#include "../../../../utils/getMetaInfoFromStages.hpp"

using namespace geode::prelude;

class StagesGraphLayer : public CCLayer
{
private:
  CCSize m_size;
  Graph *m_graph = nullptr;

  GJGameLevel *m_level = nullptr;
  std::vector<Stage> *m_stages = nullptr;

public:
  static StagesGraphLayer *create(GJGameLevel *level, const CCSize &contentSize);
  bool init(GJGameLevel *level, const CCSize &contentSize);
};