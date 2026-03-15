#include <Geode/modify/LevelInfoLayer.hpp>

#include <cmath>
#include <string>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include "../getMetaInfoFromStages.hpp"
#include "../../store/GlobalStore.hpp"
#include "../../serialization/profile/index.hpp"

using namespace geode::prelude;

void replacePracticeBarWithProgression(std::string levelId, CCNode *practiceModeBar, CCNode *practiceModeLabel, CCNode *practiceModePercentage);