#include "replacePracticeBarWithProgression.hpp"

void replacePracticeBarWithProgression(
    std::string levelId, CCNode *practiceModeBar, CCNode *_practiceModeLabel, CCNode *_practiceModePercentage)
{
  // practice-bar-stages-fill-color
  // practice-bar-ranges-fill-color
  cocos2d::ccColor3B stagesColor = Mod::get()->getSettingValue<cocos2d::ccColor3B>("practice-bar-stages-fill-color");
  cocos2d::ccColor3B rangesColor = Mod::get()->getSettingValue<cocos2d::ccColor3B>("practice-bar-ranges-fill-color");
  cocos2d::ccColor3B divisionsColor = Mod::get()->getSettingValue<cocos2d::ccColor3B>("practice-bar-divisions-color");
  int divisionsTransparency = Mod::get()->getSettingValue<float>("practice-bar-divisions-transparency") * 255;
  bool enabled = Mod::get()->getSettingValue<bool>("replace-practice-bar-with-blitzkrieg");

  if (!enabled)
    return;

  auto currentProfile = GlobalStore::get()->getProfileByLevel(levelId);

  if (!currentProfile)
    return;

  auto metaInfo = getMetaInfoFromStages(currentProfile->data.stages);
  auto practiceModeLabel = typeinfo_cast<CCLabelBMFont *>(_practiceModeLabel);
  auto practiceModePercentage = typeinfo_cast<CCLabelBMFont *>(_practiceModePercentage);

  if (!practiceModeBar || !practiceModeLabel || !practiceModePercentage)
    return;

  std::string newLabelText = fmt::format(
      "Stage {}/{} Runs {}/{}",
      std::max(metaInfo.completed, 1),
      metaInfo.total,
      metaInfo.currStageCompletedRanges,
      metaInfo.currStageTotalRanges);

  practiceModeLabel->setString("Blitzkrieg progression");
  practiceModePercentage->setString(newLabelText.c_str());
  practiceModePercentage->setScale(practiceModePercentage->getScale() * .75f);

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

    stageProgressSpr->setColor({(GLubyte)stagesColor.r, (GLubyte)stagesColor.g, (GLubyte)stagesColor.b});
    stageProgressSpr->setZOrder(1);

    // ! Ranges progression
    float rangesRatio = metaInfo.currStageCompletedRanges > 0
                            ? static_cast<float>(metaInfo.currStageCompletedRanges) / metaInfo.currStageTotalRanges
                            : 0.0f;
    float newRangeProgressWidth = newStageProgressWidth + practiceModeBar->getContentWidth() / metaInfo.total * rangesRatio;

    auto rangesProgressSpr = CCSprite::create("GJ_progressBar_001.png");
    rangesProgressSpr->setAnchorPoint({0, .5f});
    rangesProgressSpr->setPosition(stageProgressSpr->getPosition());
    rangesProgressSpr->setContentSize(stageProgressSpr->getContentSize());
    rangesProgressSpr->setScaleX(stageProgressSpr->getScaleX());
    rangesProgressSpr->setScaleY(stageProgressSpr->getScaleY());
    rangesProgressSpr->setTextureRect({0, 0, newRangeProgressWidth, practiceModeBar->getContentHeight()});
    rangesProgressSpr->setColor({(GLubyte)rangesColor.r, (GLubyte)rangesColor.g, (GLubyte)rangesColor.b});
    rangesProgressSpr->setZOrder(0);

    practiceModeBar->addChild(rangesProgressSpr);

    for (int i = 0; i < metaInfo.total - 1; i++)
    {
      auto line = CCLayerColor::create(
          {(GLubyte)divisionsColor.r, (GLubyte)divisionsColor.g, (GLubyte)divisionsColor.b, (GLubyte)divisionsTransparency},
          2,
          practiceModeBar->getContentHeight());
      line->setScaleY(stageProgressSpr->getScaleY());
      line->setPositionX(practiceModeBar->getContentWidth() / metaInfo.total * (i + 1));
      line->setZOrder(2);

      practiceModeBar->addChild(line);
    }
  }
}