#include "GoalsLayer.hpp"

#include <ctime>

#include "../../../../store/GlobalStore.hpp"

namespace
{
  const char *ACHIEVEMENTS_SEEN_KEY = "achievements-seen-at";
}

GoalsLayer *GoalsLayer::create(GJGameLevel *level, const CCSize &size)
{
  auto ret = new GoalsLayer();
  if (ret && ret->init(level, size))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool GoalsLayer::init(GJGameLevel *level, const CCSize &size)
{
  if (!CCLayer::init())
    return false;

  this->setContentSize(size);

  auto *profile = level ? GlobalStore::get()->getProfileByLevel(level) : nullptr;
  const auto seenAt = static_cast<std::time_t>(Mod::get()->getSavedValue<int64_t>(ACHIEVEMENTS_SEEN_KEY, 0));

  auto view = GoalsView::create(
      {size.width - SIDE_PADDING * 2, size.height - TOP_PADDING - BOTTOM_PADDING},
      profile,
      seenAt);
  view->setPosition({SIDE_PADDING, BOTTOM_PADDING});
  this->addChild(view);

  // The view already took the old value for its New marks
  Mod::get()->setSavedValue<int64_t>(ACHIEVEMENTS_SEEN_KEY, static_cast<int64_t>(std::time(nullptr)));

  return true;
}
