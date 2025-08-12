#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/binding/EditLevelLayer.hpp>

using namespace geode::prelude;

class $modify(DTEditLevelLayer, EditLevelLayer) {
  public:
    bool init(GJGameLevel* p0);
};