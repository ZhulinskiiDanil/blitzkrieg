#include "index.hpp"
#include "../../utils/loadProfiles.hpp"

bool DTEditLevelLayer::init(GJGameLevel* p0) {
  if (!EditLevelLayer::init(p0)) return false;
  geode::log::debug("EDIT LAYER");

  loadProfiles();

  return true;
}