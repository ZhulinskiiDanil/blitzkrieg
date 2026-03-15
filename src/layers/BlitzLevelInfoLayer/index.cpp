#include "./index.hpp"

bool BlitzLevelInfoLayer::init(GJGameLevel *level, bool challenge)
{
    if (!LevelInfoLayer::init(level, challenge))
        return false;

    bool enabled = Mod::get()->getSettingValue<bool>("replace-practice-bar-with-blitzkrieg");

    if (!enabled)
        return true;

    m_fields->currentProfile = GlobalStore::get()->getProfileByLevel(level);

    if (m_fields->currentProfile.id.empty())
        return true;

    auto metaInfo = getMetaInfoFromStages(m_fields->currentProfile.data.stages);
    auto practiceModeBar = this->getChildByID("practice-mode-bar");
    auto practiceModeLabel = typeinfo_cast<CCLabelBMFont *>(this->getChildByID("practice-mode-label"));
    auto practiceModePercentage = typeinfo_cast<CCLabelBMFont *>(this->getChildByID("practice-mode-percentage"));

    if (!practiceModeBar || !practiceModeLabel || !practiceModePercentage)
        return true;

    std::string newLabelText = fmt::format(
        "Stage {}/{} Runs {}/{}",
        std::max(metaInfo.completed, 1),
        metaInfo.total,
        metaInfo.currStageCompletedRanges,
        metaInfo.currStageTotalRanges);

    practiceModeLabel->setString("Blitzkrieg progression");
    practiceModePercentage->setString(newLabelText.c_str());
    practiceModePercentage->setScale(.3f);

    if (auto stageProgressSpr = typeinfo_cast<CCSprite *>(practiceModeBar->getChildByIndex(0)))
    {
        // ! Stages progression
        auto sprSize = stageProgressSpr->getContentSize();
        float stagesRatio = metaInfo.completed > 0
                                ? static_cast<float>(metaInfo.completed) / metaInfo.total
                                : 0.0f;
        float newStageProgressWidth = practiceModeBar->getContentWidth() * stagesRatio;

        stageProgressSpr->setContentWidth(newStageProgressWidth);
        stageProgressSpr->setTextureRect({0, 0, newStageProgressWidth, practiceModeBar->getContentHeight()});
        stageProgressSpr->setColor({255, 0, 82});
        stageProgressSpr->setZOrder(1);

        // ! Ranges progression
        float rangesRatio = metaInfo.currStageCompletedRanges > 0
                                ? static_cast<float>(metaInfo.currStageCompletedRanges) / metaInfo.currStageTotalRanges
                                : 0.0f;
        float newRangeProgressWidth = practiceModeBar->getContentWidth() / metaInfo.total * stagesRatio + newStageProgressWidth;

        auto rangesProgressSpr = CCSprite::create("GJ_progressBar_001.png");
        rangesProgressSpr->setAnchorPoint({0, .5f});
        rangesProgressSpr->setPosition(stageProgressSpr->getPosition());
        rangesProgressSpr->setContentSize(stageProgressSpr->getContentSize());
        rangesProgressSpr->setScaleX(stageProgressSpr->getScaleX());
        rangesProgressSpr->setScaleY(stageProgressSpr->getScaleY());
        rangesProgressSpr->setTextureRect({0, 0, newRangeProgressWidth, practiceModeBar->getContentHeight()});
        rangesProgressSpr->setColor({255, 200, 207});
        rangesProgressSpr->setZOrder(0);

        practiceModeBar->addChild(rangesProgressSpr);

        for (int i = 0; i < metaInfo.total - 1; i++)
        {
            auto line = CCLayerColor::create({255, 255, 255, 100}, 2, practiceModeBar->getContentHeight());
            line->setScaleY(stageProgressSpr->getScaleY());
            line->setPositionX(practiceModeBar->getContentWidth() / metaInfo.total * (i + 1));
            line->setZOrder(2);

            practiceModeBar->addChild(line);
        }
    }

    return true;
}