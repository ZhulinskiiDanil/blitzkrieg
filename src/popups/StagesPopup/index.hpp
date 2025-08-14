#pragma once

#include <string>

#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <nlohmann/json.hpp>

#include <Geode/Geode.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/extensions/GUI/CCControlExtension/CCScale9Sprite.h>

#include "../../serialization/profile/index.hpp"
#include "../../utils/getProfiles.hpp"
#include "../../utils/saveProfiles.hpp"
#include "../../utils/linkProfileWithLevel.hpp"
#include "../../utils/selectJsonFile.hpp"

using namespace geode::prelude;
using json = nlohmann::json;

struct Padding
{
  float top;
  float bottom;
  float left;
  float right;
};

// class StagesPopupDelegate {
// public:
//   virtual void onFolderCreate(std::string);
// };

class StagesPopup : public geode::Popup<GJGameLevel *>
{
private:
  GJGameLevel *m_level = nullptr;

  bool setup(GJGameLevel *) override;
  void onProfileSelect(CCObject *);
  void onImport(CCObject *);
  void onExport(CCObject *);

public:
  // static StagesPopup* create(StagesPopupDelegate*);
  static StagesPopup *create(GJGameLevel *);
};