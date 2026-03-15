#include <Geode/modify/LevelInfoLayer.hpp>

#include <cmath>
#include <string>
#include "../../store/GlobalStore.hpp"
#include "../../serialization/profile/index.hpp"
#include "../../utils/getMetaInfoFromStages.hpp"

using namespace geode::prelude;

class $modify(BlitzLevelInfoLayer, LevelInfoLayer)
{
    struct Fields
    {
        Profile currentProfile;
    };

public:
    bool init(GJGameLevel *level, bool challenge);
};
