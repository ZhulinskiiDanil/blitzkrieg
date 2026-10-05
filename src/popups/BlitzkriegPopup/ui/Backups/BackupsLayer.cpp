#include "BackupsLayer.hpp"

#include <algorithm>
#include <fmt/chrono.h>

namespace
{
  const ccColor4B ACCENT_COLOR{255, 0, 82, 255};
  const ccColor4B BUTTON_COLOR{45, 45, 45, 255};
  const ccColor4B CHIP_COLOR{36, 36, 36, 255};
  const ccColor4B LIST_COLOR{30, 30, 30, 255};

  // Oct 05, 18:40
  std::string formatDate(std::time_t time)
  {
    if (time <= 0)
      return "-";

    return fmt::format("{:%b %d, %H:%M}", geode::localtime(time));
  }

  CCMenuItemSpriteExtra *createPill(
      const char *text,
      float width,
      float height,
      ccColor4B color,
      CCObject *target,
      SEL_MenuHandler selector)
  {
    const CCSize size{width, height};

    auto content = CCNode::create();
    content->setContentSize(size);
    content->addChild(RectNode::create(size, ccc4FFromccc4B(color), size.height / 2));

    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->limitLabelWidth(width - 10.f, .28f, .1f);
    label->setPosition(size / 2);
    content->addChild(label);

    auto item = CCMenuItemSpriteExtra::create(content, target, selector);
    item->m_scaleMultiplier = 1.05f;
    return item;
  }
}

BackupsLayer *BackupsLayer::create(const CCSize &size)
{
  auto ret = new BackupsLayer();
  if (ret && ret->init(size))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool BackupsLayer::init(const CCSize &size)
{
  if (!CCLayer::init())
    return false;

  m_size = size;
  this->setContentSize(size);

  // ! --- Layout, top to bottom --- !
  const float controlsY = m_size.height - TOP_PADDING - CONTROLS_HEIGHT / 2;
  const float listTop = controlsY - CONTROLS_HEIGHT / 2 - ROW_GAP;

  drawControls(controlsY);
  drawList(listTop);
  rebuildList();

  return true;
}

// ! --- Controls: summary on the left, buttons on the right --- !

void BackupsLayer::drawControls(float y)
{
  const float gap = 4.f;
  const float backUpWidth = 74.f;
  const float folderWidth = 64.f;
  const float buttonsLeft = m_size.width - SIDE_PADDING - backUpWidth - folderWidth - gap;

  auto menu = CCMenu::create();
  menu->setPosition({buttonsLeft, y});
  this->addChild(menu);

  auto folder = createPill("Open folder", folderWidth, CONTROLS_HEIGHT, BUTTON_COLOR, this, menu_selector(BackupsLayer::onOpenFolder));
  folder->setPosition({folderWidth / 2, 0.f});
  menu->addChild(folder);

  auto backUp = createPill("Back up now", backUpWidth, CONTROLS_HEIGHT, ACCENT_COLOR, this, menu_selector(BackupsLayer::onBackUpNow));
  backUp->setPosition({folderWidth + gap + backUpWidth / 2, 0.f});
  menu->addChild(backUp);

  m_summary = CCNode::create();
  m_summary->setPosition({SIDE_PADDING, y});
  this->addChild(m_summary);

  m_summaryMaxWidth = buttonsLeft - 8.f - SIDE_PADDING;
}

void BackupsLayer::updateSummary(std::vector<BackupInfo> const &backups)
{
  m_summary->removeAllChildrenWithCleanup(true);
  m_summary->setScale(1.f);

  std::vector<std::string> chips;

  chips.push_back(fmt::format("{} {}", backups.size(), backups.size() == 1 ? "backup" : "backups"));

  if (Mod::get()->getSettingValue<bool>("auto-backups"))
  {
    chips.push_back(fmt::format("Auto every {}h", Mod::get()->getSettingValue<int64_t>("backups-interval")));
    chips.push_back(fmt::format("Keeps {} auto", Mod::get()->getSettingValue<int64_t>("backups-keep")));
  }
  else
  {
    chips.push_back("Auto backups off");
  }

  const float chipPadding = 5.f;
  const float chipGap = 4.f;
  float x = 0.f;

  for (auto const &text : chips)
  {
    auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    label->setScale(.24f);
    label->setOpacity(200);

    const float width = label->getScaledContentWidth() + chipPadding * 2;

    auto chip = RectNode::create({width, CONTROLS_HEIGHT}, ccc4FFromccc4B(CHIP_COLOR), CONTROLS_HEIGHT / 2);
    chip->setPosition({x, -CONTROLS_HEIGHT / 2});
    m_summary->addChild(chip);

    label->setPosition({x + width / 2, 0.f});
    m_summary->addChild(label);

    x += width + chipGap;
  }

  const float width = x - chipGap;

  if (width > m_summaryMaxWidth && width > 0.f)
    m_summary->setScale(m_summaryMaxWidth / width);
}

// ! --- List --- !

void BackupsLayer::drawList(float top)
{
  const CCSize listSize{m_size.width - SIDE_PADDING * 2, top - BOTTOM_PADDING};
  const CCPoint listCenter{SIDE_PADDING + listSize.width / 2, BOTTOM_PADDING + listSize.height / 2};

  auto background = RectNode::create(listSize, ccc4FFromccc4B(LIST_COLOR), 8.f);
  background->ignoreAnchorPointForPosition(false);
  background->setAnchorPoint({.5f, .5f});
  background->setPosition(listCenter);
  this->addChild(background, -1);

  m_scroll = ScrollLayer::create({listSize.width - 8.f, listSize.height - 8.f});
  m_scroll->setPosition({SIDE_PADDING + 4.f, BOTTOM_PADDING + 4.f});
  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(4.f)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setCrossAxisAlignment(AxisAlignment::Center)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));
  this->addChild(m_scroll);

  // ! --- Borders, the same as the other lists --- !
  auto borders = ListBorders::create();
  borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f);
  borders->setContentSize({listSize.width, listSize.height - 3.f});
  borders->setAnchorPoint({.5f, .5f});
  borders->setPosition(listCenter - CCPoint{0.f, .5f});
  borders->updateLayout();
  this->addChild(borders);

  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
    child->setColor(ccc3(50, 50, 50));

  // ! --- Empty state --- !
  m_emptyState = CCNode::create();
  m_emptyState->setPosition(listCenter);
  m_emptyState->setVisible(false);
  this->addChild(m_emptyState, 1);

  auto title = CCLabelBMFont::create("No backups yet", "bigFont.fnt");
  title->setScale(.45f);
  title->setOpacity(170);
  title->setPosition({0.f, 8.f});
  m_emptyState->addChild(title);

  auto hint = CCLabelBMFont::create("Press \"Back up now\" or wait for an automatic one", "bigFont.fnt");
  hint->limitLabelWidth(listSize.width - 30.f, .28f, .1f);
  hint->setOpacity(110);
  hint->setPosition({0.f, -8.f});
  m_emptyState->addChild(hint);
}

void BackupsLayer::rebuildList()
{
  if (!m_scroll)
    return;

  auto *contentLayer = m_scroll->m_contentLayer;
  contentLayer->removeAllChildrenWithCleanup(true);

  const auto backups = BackupStore::get()->list();

  updateSummary(backups);
  m_emptyState->setVisible(backups.empty());

  const float width = m_scroll->getContentWidth();

  for (auto const &backup : backups)
  {
    auto cell = BackupCell::create(backup, width);
    cell->onRestoreRequested = [this](BackupInfo const &info)
    {
      restore(info);
    };
    cell->onDeleteRequested = [this](BackupInfo const &info)
    {
      remove(info);
    };

    contentLayer->addChild(cell);
  }

  contentLayer->updateLayout();
  m_scroll->scrollToTop();
}

void BackupsLayer::queueRebuild()
{
  if (m_rebuildQueued)
    return;

  m_rebuildQueued = true;

  geode::queueInMainThread(
      [self = Ref<BackupsLayer>(this)]()
      {
        self->m_rebuildQueued = false;

        // The popup was closed before the rebuild ran
        if (!self->getParent())
          return;

        self->rebuildList();
      });
}

// ! --- Actions --- !

void BackupsLayer::restore(BackupInfo const &info)
{
  geode::createQuickPopup(
      "Restore Backup",
      fmt::format(
          "All profiles will be <cr>replaced</c> with the backup from <cy>{}</c>.\n"
          "The current profiles are backed up first, so this can be undone.",
          formatDate(info.createdAt)),
      "Cancel",
      "Restore",
      [self = Ref<BackupsLayer>(this), id = info.id](auto, bool confirmed)
      {
        if (!confirmed)
          return;

        auto result = BackupStore::get()->restore(id);

        if (result.isErr())
        {
          Notification::create(
              fmt::format("Restore failed: {}", result.unwrapErr()),
              NotificationIcon::Error)
              ->show();
        }
        else
        {
          Notification::create("Backup restored", NotificationIcon::Success)->show();
        }

        self->queueRebuild();
      });
}

void BackupsLayer::remove(BackupInfo const &info)
{
  geode::createQuickPopup(
      "Delete Backup",
      fmt::format("The backup from <cy>{}</c> will be <cr>deleted</c>.", formatDate(info.createdAt)),
      "Cancel",
      "Delete",
      [self = Ref<BackupsLayer>(this), id = info.id](auto, bool confirmed)
      {
        if (!confirmed)
          return;

        if (auto result = BackupStore::get()->remove(id); result.isErr())
        {
          Notification::create(
              fmt::format("Delete failed: {}", result.unwrapErr()),
              NotificationIcon::Error)
              ->show();
        }

        self->queueRebuild();
      });
}

void BackupsLayer::onBackUpNow(CCObject *)
{
  auto result = BackupStore::get()->create(BackupReason::Manual);

  if (result.isErr())
  {
    Notification::create(result.unwrapErr(), NotificationIcon::Warning)->show();
    return;
  }

  Notification::create("Profiles backed up", NotificationIcon::Success)->show();
  queueRebuild();
}

void BackupsLayer::onOpenFolder(CCObject *)
{
  const auto dir = BackupStore::get()->getDir();

  std::error_code ec;
  std::filesystem::create_directories(dir, ec);

  file::openFolder(dir);
}
