#include <Geode/modify/LevelInfoLayer.hpp>
#include "../../utils/ui/replacePracticeBarWithProgression.hpp"

using namespace geode::prelude;

class $modify(BlitzLevelInfoLayer, LevelInfoLayer)
{
public:
    bool init(GJGameLevel *level, bool challenge);
};
