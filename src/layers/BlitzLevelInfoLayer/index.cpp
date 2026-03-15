#include "./index.hpp"

bool BlitzLevelInfoLayer::init(GJGameLevel *level, bool challenge)
{
    if (!LevelInfoLayer::init(level, challenge))
        return false;

    auto practiceModeBar = this->getChildByID("practice-mode-bar");
    auto practiceModeLabel = this->getChildByID("practice-mode-label");
    auto practiceModePercentage = this->getChildByID("practice-mode-percentage");

    replacePracticeBarWithProgression(utils::numToString(level->m_levelID.value()), practiceModeBar, practiceModeLabel, practiceModePercentage);

    return true;
}