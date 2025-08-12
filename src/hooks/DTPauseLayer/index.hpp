#include <Geode/modify/PauseLayer.hpp>
#include "../../popups/StagesPopup/index.hpp"

using namespace geode::prelude;

// class $modify(DTPauseLayer, PauseLayer), public CreateFolderDelegate {
class $modify(DTPauseLayer, PauseLayer)
{
  void customSetup();
  void onImport(CCObject *);
  void onPopup(CCObject *);

  // void onFolderCreate(std::string name) override;
};