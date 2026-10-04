#include "ProfilesListLayer.hpp"

#include <algorithm>
#include <numeric>

ProfilesListLayer *ProfilesListLayer::create(
    GJGameLevel *level,
    std::vector<Profile> const &profiles,
    const CCSize &contentSize)
{
  auto ret = new ProfilesListLayer();
  if (ret && ret->init(level, profiles, contentSize))
  {
    ret->autorelease();

    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool ProfilesListLayer::init(
    GJGameLevel *level,
    std::vector<Profile> const &profiles,
    const CCSize &contentSize)
{
  if (!CCLayer::init())
    return false;

  m_contentSize = contentSize;
  m_level = level;
  m_profiles = profiles;

  this->setContentSize(m_contentSize);

  float padding = 5.f;

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(contentSize);
  m_scroll->setContentSize({contentSize.width - padding * 2, contentSize.height - padding * 2});
  m_scroll->setPosition({padding, padding});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(2.5f)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));

  this->addChild(m_scroll);

  // ! --- BG --- !
  RectNode *bg = RectNode::create(contentSize, ccc4FFromccc4B({30, 30, 30, 255}), 8);
  bg->ignoreAnchorPointForPosition(false);
  bg->setAnchorPoint({0.5f, 0.5f});
  bg->setPosition(contentSize / 2);
  bg->setZOrder(-1);
  this->addChild(bg);

  // ! --- Borders --- !
  auto borders = ListBorders::create();
  borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f); // 2.1f
  borders->updateLayout();
  borders->setContentSize({contentSize.width, contentSize.height - 3});
  borders->setPosition({contentSize.width / 2, contentSize.height / 2 - .5f});
  borders->setAnchorPoint({0.5f, 0.5f});
  this->addChild(borders);

  // ! Set borders color to dark gray
  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
    child->setColor(ccc3(50, 50, 50));

  createEmptyState();

  // ! --- Bottom buttons --- !
  auto btnsGap = 5.f;

  auto btnSprCreate = ButtonSprite::create("Create");
  auto btnCreate = CCMenuItemSpriteExtra::create(
      btnSprCreate, this, menu_selector(ProfilesListLayer::onCreate));
  btnCreate->setScale(.75f);
  btnCreate->m_baseScale = .75f;
  btnCreate->ignoreAnchorPointForPosition(true);

  auto btnSprImport = ButtonSprite::create("Import");
  auto btnImport = CCMenuItemSpriteExtra::create(
      btnSprImport, this, menu_selector(ProfilesListLayer::onImport));
  btnImport->setScale(.75f);
  btnImport->m_baseScale = .75f;
  btnImport->ignoreAnchorPointForPosition(true);

  auto btnSprExport = ButtonSprite::create("Export");
  auto btnExport = CCMenuItemSpriteExtra::create(
      btnSprExport, this, menu_selector(ProfilesListLayer::onExport));
  btnExport->setScale(.75f);
  btnExport->m_baseScale = .75f;
  btnExport->ignoreAnchorPointForPosition(true);

  auto btnMenu = CCMenu::create();
  btnMenu->setLayout(
      RowLayout::create()
          ->setGap(btnsGap)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center));

  btnMenu->addChild(btnCreate);
  btnMenu->addChild(btnImport);
  btnMenu->addChild(btnExport);
  btnMenu->setAnchorPoint({0.f, .5f});
  btnMenu->setPosition({0.f, -20.f});

  this->addChild(btnMenu);
  btnMenu->updateLayout();

  // ! --- Events --- !
  m_profilesListener = ProfilesChangedEvent().listen(
      [this]()
      {
        queueReload();
        return ListenerResult::Propagate;
      });

  // Linking changes the order and the highlight of rows
  m_profileListener = ProfileChangedEvent().listen(
      [this]()
      {
        queueReload();
        return ListenerResult::Propagate;
      });

  reload();
  return true;
}

void ProfilesListLayer::createEmptyState()
{
  m_emptyState = CCNode::create();
  m_emptyState->setPosition(m_contentSize / 2);
  m_emptyState->setVisible(false);
  this->addChild(m_emptyState, 1);

  auto title = CCLabelBMFont::create("No profiles yet", "bigFont.fnt");
  title->setScale(.5f);
  title->setOpacity(200);
  title->setPosition({0.f, 8.f});
  m_emptyState->addChild(title);

  auto hint = CCLabelBMFont::create(
      "Create one for this level or import a backup",
      "bigFont.fnt");
  hint->setScale(.3f);
  hint->setOpacity(120);
  hint->setPosition({0.f, -8.f});
  hint->limitLabelWidth(m_contentSize.width - 20.f, .3f, .15f);
  m_emptyState->addChild(hint);
}

void ProfilesListLayer::queueReload(bool keepScroll)
{
  // If any request in this frame wants the top, go to the top
  m_queuedKeepScroll = m_reloadQueued
                           ? m_queuedKeepScroll && keepScroll
                           : keepScroll;

  if (m_reloadQueued)
    return;

  m_reloadQueued = true;

  geode::queueInMainThread(
      [self = Ref<ProfilesListLayer>(this)]()
      {
        self->m_reloadQueued = false;

        // The popup was closed before the reload ran
        if (!self->getParent())
          return;

        self->m_profiles = GlobalStore::get()->getProfiles();
        self->reload(self->m_queuedKeepScroll);
      });
}

void ProfilesListLayer::reload(bool keepScroll)
{
  if (!m_level)
    return;

  auto *content = m_scroll->m_contentLayer;
  const float viewHeight = m_scroll->getContentHeight();

  // Content layer is at (viewHeight - contentHeight) when scrolled to the top
  const float distanceFromTop =
      content->getPositionY() - (viewHeight - content->getContentHeight());

  content->removeAllChildrenWithCleanup(true);

  // ! --- Order: linked, pinned, others; store order inside each group --- !
  const auto *linkedProfile = GlobalStore::get()->getProfileByLevel(m_level);
  const std::string linkedId = linkedProfile ? linkedProfile->id : "";

  std::vector<int> groups(m_profiles.size());

  for (std::size_t i = 0; i < m_profiles.size(); ++i)
  {
    const auto &id = m_profiles[i].id;

    if (!linkedId.empty() && id == linkedId)
      groups[i] = 0;
    else if (GlobalStore::get()->isProfilePinned(id))
      groups[i] = 1;
    else
      groups[i] = 2;
  }

  std::vector<std::size_t> order(m_profiles.size());
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::stable_sort(
      order.begin(),
      order.end(),
      [&groups](std::size_t a, std::size_t b)
      {
        return groups[a] < groups[b];
      });

  // ! --- Rows --- !
  for (auto index : order)
  {
    auto profileItem = BlitzkriegProfile::create(
        m_profiles[index],
        m_level,
        CCSize(m_scroll->getContentWidth(), 40.f));

    if (profileItem)
      content->addChild(profileItem);
  }

  if (m_emptyState)
    m_emptyState->setVisible(m_profiles.empty());

  content->updateLayout();

  // ! --- Scroll position --- !
  const float maxDistance =
      std::max(0.f, content->getContentHeight() - viewHeight);

  const float distance =
      keepScroll ? std::clamp(distanceFromTop, 0.f, maxDistance) : 0.f;

  content->setPositionY(viewHeight - content->getContentHeight() + distance);
}

void ProfilesListLayer::scrollToTop()
{
  if (m_scroll)
    m_scroll->scrollToTop();
}

void ProfilesListLayer::onImport(CCObject *obj)
{
  selectJsonFile(
      [](std::string jsonContent)
      {
        if (jsonContent.empty())
          return;

        auto res = matjson::parseAs<std::vector<Profile>>(jsonContent);

        if (res.isErr())
        {
          geode::log::error("JSON parse error: {}", res.unwrapErr());

          FLAlertLayer::create(
              "Import Error",
              fmt::format("Failed to import profiles: {}", res.unwrapErr()),
              "OK")
              ->show();

          return;
        }

        auto profiles = res.unwrap();

        if (profiles.empty())
        {
          geode::log::error("Imported JSON does not contain any profiles");

          FLAlertLayer::create(
              "Import Error",
              "The file does not contain any profiles",
              "OK")
              ->show();

          return;
        }

        const std::size_t added = GlobalStore::get()->addProfiles(profiles);
        const std::size_t skipped = profiles.size() - added;

        auto plural = [](std::size_t count)
        {
          return count == 1 ? "profile" : "profiles";
        };

        if (added == 0)
        {
          Notification::create(
              profiles.size() == 1
                  ? std::string("This profile already exists")
                  : fmt::format("All {} profiles already exist", profiles.size()),
              NotificationIcon::Info)
              ->show();

          return;
        }

        std::string message = fmt::format("Imported {} {}", added, plural(added));

        if (skipped > 0)
          message += fmt::format(", {} skipped (already exist)", skipped);

        Notification::create(message, NotificationIcon::Success)->show();

        // The list reloads itself through the event
        ProfilesChangedEvent().send();
      });
}

void ProfilesListLayer::onExport(CCObject *obj)
{
  auto const &profiles = GlobalStore::get()->getProfiles();

  WhiteListExport::create(profiles)->show();
}

void ProfilesListLayer::onCreate(CCObject *sender)
{
  if (!m_level)
    return;

  std::vector<float> percentages = findStartposesFromCurrentLevel().percentages_2_1;
  bool isSPCountValid = percentages.size() <= 8;

  if (isSPCountValid)
  {
    createQuickPopup(
        "Too few startposes",
        "It is <cg>recommended</c> to use more than <cc>8 startposes</c>. If you still want to <cg>create a profile</c>, click <cy>Continue</c>.",
        "Back", "Continue",
        [this](auto, bool onContinueBtn)
        {
          if (onContinueBtn)
          {
            const auto createProfilePopup = CreateProfilePopup::create(m_level);
            createProfilePopup->show();
          }
        });
  }
  else
  {
    const auto createProfilePopup = CreateProfilePopup::create(m_level);
    createProfilePopup->show();
  }
}
