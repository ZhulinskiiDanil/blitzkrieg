#include "BackupCell.hpp"

#include <fmt/chrono.h>

namespace
{
  const ccColor4B CELL_COLOR{40, 40, 40, 255};
  const ccColor4B BUTTON_COLOR{55, 55, 55, 255};

  // 12.4 KB, 1.2 MB
  std::string formatSize(std::uintmax_t bytes)
  {
    if (bytes < 1024)
      return fmt::format("{} B", bytes);

    if (bytes < 1024 * 1024)
      return fmt::format("{:.1f} KB", bytes / 1024.0);

    return fmt::format("{:.1f} MB", bytes / (1024.0 * 1024.0));
  }

  // Oct 05, 18:40, with the year when it is not this year
  std::string formatDate(std::time_t time)
  {
    if (time <= 0)
      return "-";

    const auto local = geode::localtime(time);
    const auto now = geode::localtime(std::time(nullptr));

    if (local.tm_year == now.tm_year)
      return fmt::format("{:%b %d, %H:%M}", local);

    return fmt::format("{:%b %d %Y, %H:%M}", local);
  }

  // The same rule as the profile list
  std::string getDisplayName(std::string name)
  {
    if (Mod::get()->getSettingValue<bool>("enable-streamer-mode") && !name.empty())
      name = name.substr(0, 1) + "...";

    return name;
  }
}

ccColor3B getBackupReasonColor(BackupReason reason)
{
  switch (reason)
  {
  case BackupReason::Manual:
    return {255, 0, 82};
  case BackupReason::BeforeDelete:
    return {253, 106, 106};
  case BackupReason::BeforeImport:
    return {120, 180, 255};
  case BackupReason::BeforeStartposChange:
    return {255, 196, 157};
  case BackupReason::BeforeRestore:
    return {255, 220, 90};
  default:
    return {150, 150, 150};
  }
}

BackupCell *BackupCell::create(BackupInfo const &info, float width)
{
  auto ret = new BackupCell();
  if (ret && ret->init(info, width))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool BackupCell::init(BackupInfo const &info, float width)
{
  if (!CCNode::init())
    return false;

  m_info = info;
  this->setContentSize({width, HEIGHT});

  this->addChild(RectNode::create({width, HEIGHT}, ccc4FFromccc4B(CELL_COLOR), 5.f));

  // ! --- Buttons on the right, the text stays left of them --- !
  auto menu = CCMenu::create();
  menu->setPosition({0.f, 0.f});
  this->addChild(menu, 1);

  const CCSize restoreSize{50.f, 16.f};

  auto restoreContent = CCNode::create();
  restoreContent->setContentSize(restoreSize);
  restoreContent->addChild(RectNode::create(restoreSize, ccc4FFromccc4B(BUTTON_COLOR), restoreSize.height / 2));

  auto restoreLabel = CCLabelBMFont::create("Restore", "bigFont.fnt");
  restoreLabel->limitLabelWidth(restoreSize.width - 10.f, .28f, .1f);
  restoreLabel->setColor({99, 224, 110});
  restoreLabel->setPosition(restoreSize / 2);
  restoreContent->addChild(restoreLabel);

  auto restore = CCMenuItemSpriteExtra::create(restoreContent, this, menu_selector(BackupCell::onRestore));
  restore->m_scaleMultiplier = 1.05f;

  auto trashSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
  trashSprite->setScale(.5f);

  auto trash = CCMenuItemSpriteExtra::create(trashSprite, this, menu_selector(BackupCell::onDelete));
  trash->setPosition({width - PADDING - trash->getContentWidth() / 2, HEIGHT / 2});
  menu->addChild(trash);

  const float restoreRight = width - PADDING - trash->getContentWidth() - 6.f;
  restore->setPosition({restoreRight - restoreSize.width / 2, HEIGHT / 2});
  menu->addChild(restore);

  const float textRight = restoreRight - restoreSize.width - 8.f;

  // ! --- First line: date and a reason chip --- !
  auto date = CCLabelBMFont::create(formatDate(info.createdAt).c_str(), "bigFont.fnt");
  date->setAnchorPoint({0.f, .5f});
  date->limitLabelWidth(textRight - PADDING, .36f, .2f);
  date->setPosition({PADDING, HEIGHT - 11.f});
  this->addChild(date);

  const auto reasonColor = getBackupReasonColor(info.reason);
  const float chipX = PADDING + date->getScaledContentWidth() + 6.f;

  auto reasonLabel = CCLabelBMFont::create(getBackupReasonName(info.reason), "bigFont.fnt");
  reasonLabel->setScale(.22f);
  reasonLabel->setColor(reasonColor);

  const CCSize chipSize{reasonLabel->getScaledContentWidth() + 10.f, 11.f};

  if (chipX + chipSize.width <= textRight)
  {
    auto chipColor = ccc4FFromccc3B(reasonColor);
    chipColor.a = .15f;

    auto chip = RectNode::create(chipSize, premultiplyAlpha(chipColor), chipSize.height / 2);
    chip->setPosition({chipX, HEIGHT - 11.f - chipSize.height / 2});
    this->addChild(chip);

    reasonLabel->setPosition({chipX + chipSize.width / 2, HEIGHT - 11.f});
    this->addChild(reasonLabel);
  }

  // ! --- Second line: note, profiles and size --- !
  std::vector<std::string> parts;

  if (!info.note.empty())
  {
    // A restore notes the backup id, other reasons a profile name
    parts.push_back(info.reason == BackupReason::BeforeRestore
                        ? fmt::format("Restoring {}", info.note)
                        : getDisplayName(info.note));
  }

  if (!info.profileNames.empty())
  {
    std::string names;
    const std::size_t shown = std::min<std::size_t>(info.profileNames.size(), 3);

    for (std::size_t i = 0; i < shown; ++i)
      names += (i ? ", " : "") + getDisplayName(info.profileNames[i]);

    if (info.profileNames.size() > shown)
      names += fmt::format(" +{}", info.profileNames.size() - shown);

    parts.push_back(fmt::format(
        "{} {}: {}",
        info.profileNames.size(),
        info.profileNames.size() == 1 ? "profile" : "profiles",
        names));
  }

  parts.push_back(formatSize(info.size));

  std::string details;

  for (std::size_t i = 0; i < parts.size(); ++i)
    details += (i ? "   |   " : "") + parts[i];

  auto detailsLabel = CCLabelBMFont::create(details.c_str(), "chatFont.fnt");
  detailsLabel->setAnchorPoint({0.f, .5f});
  detailsLabel->limitLabelWidth(textRight - PADDING, .5f, .2f);
  detailsLabel->setOpacity(160);
  detailsLabel->setPosition({PADDING, 10.f});
  this->addChild(detailsLabel);

  return true;
}

void BackupCell::onRestore(CCObject *)
{
  if (onRestoreRequested)
    onRestoreRequested(m_info);
}

void BackupCell::onDelete(CCObject *)
{
  if (onDeleteRequested)
    onDeleteRequested(m_info);
}
