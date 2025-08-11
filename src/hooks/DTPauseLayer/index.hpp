#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(DTPauseLayer, PauseLayer) {
  void customSetup();

  void onDTMiniMenuOpened(CCObject*);
};