#include "WhiteListExport.hpp"

WhiteListExport *WhiteListExport::create(std::vector<Profile> const &profiles)
{
  auto ret = new WhiteListExport();

  if (ret && ret->init(profiles))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool WhiteListExport::init(std::vector<Profile> const &profiles)
{
  if (!Popup::init(380.f, 280.f, "GJ_square01_custom.png"_spr))
    return false;

  m_profiles = profiles;
  m_selected.assign(m_profiles.size(), true);

  createTitleRow();
  createProfilesList();
  createOptionCheckboxes();
  createWarning();
  createBottomButtons();

  return true;
}

// ! --- Layout --- !

void WhiteListExport::createTitleRow()
{
  float const rowY = m_size.height - 22.f;

  // ! --- Title --- !
  setTitle("Export Profiles", "goldFont.fnt", .5f, 20.f);
  m_title->limitLabelWidth(180.f, .5f, .1f);
  m_title->setAnchorPoint({0.f, .5f});
  m_title->setPosition({12.f, rowY});

  // ! --- Buttons --- !
  auto menu = CCMenu::create();
  menu->setContentSize({m_size.width - 24.f, 16.f});
  menu->setAnchorPoint({1.f, .5f});
  menu->setPosition({m_size.width - 12.f, rowY});
  menu->setLayout(
      RowLayout::create()
          ->setGap(6.f)
          ->setAutoScale(false)
          ->setAxisAlignment(AxisAlignment::End));

  auto selectAllSpr = ButtonSprite::create(
      "Select All", 0, 0, "bigFont.fnt", "GJ_button_04.png", 0.f, .5f);
  auto selectAllBtn = CCMenuItemSpriteExtra::create(
      selectAllSpr, this, menu_selector(WhiteListExport::onSelectAll));
  selectAllBtn->setScale(.6f);
  selectAllBtn->m_baseScale = .6f;
  selectAllBtn->ignoreAnchorPointForPosition(true);

  auto deselectAllSpr = ButtonSprite::create(
      "Deselect All", 0, 0, "bigFont.fnt", "GJ_button_04.png", 0.f, .5f);
  auto deselectAllBtn = CCMenuItemSpriteExtra::create(
      deselectAllSpr, this, menu_selector(WhiteListExport::onDeselectAll));
  deselectAllBtn->setScale(.6f);
  deselectAllBtn->m_baseScale = .6f;
  deselectAllBtn->ignoreAnchorPointForPosition(true);

  menu->addChild(selectAllBtn);
  menu->addChild(deselectAllBtn);

  m_mainLayer->addChild(menu);
  menu->updateLayout();
}

void WhiteListExport::createProfilesList()
{
  float const scrollWidth = m_size.width - 24.f;
  float const scrollBottomY = 96.f;
  float const scrollTop = m_size.height - 40.f;
  float const scrollHeight = scrollTop - scrollBottomY;

  CCSize const scrollSize{scrollWidth, scrollHeight};

  // ! --- Background --- !
  auto bg = CCScale9Sprite::create("range-default-bg.png"_spr);
  bg->setContentSize(scrollSize);
  bg->setColor({17, 16, 16});
  bg->setOpacity(255 * .3f);
  bg->ignoreAnchorPointForPosition(true);
  bg->setPosition({12.f, scrollBottomY});
  bg->setZOrder(-1);
  m_mainLayer->addChild(bg);

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(scrollSize);
  m_scroll->setPosition({12.f, scrollBottomY});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(2.f)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));

  m_mainLayer->addChild(m_scroll);

  // ! --- Borders --- !
  auto borders = ListBorders::create();
  borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f);

  borders->setContentSize({
      scrollSize.width,
      scrollSize.height - 3.f,
  });

  borders->setAnchorPoint({0.5f, 0.5f});

  borders->setPosition({
      m_size.width / 2.f,
      scrollBottomY + scrollHeight / 2.f - 0.5f,
  });

  borders->updateLayout();
  m_mainLayer->addChild(borders);

  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
    child->setColor(ccc3(50, 50, 50));

  // ! --- Profile cells --- !
  if (m_profiles.empty())
  {
    auto emptyLabel = CCLabelBMFont::create(
        "No profiles to export",
        "bigFont.fnt");

    emptyLabel->setScale(.4f);
    emptyLabel->setOpacity(255 * .5f);
    emptyLabel->setAnchorPoint({.5f, .5f});
    emptyLabel->setPosition(scrollSize / 2.f);

    m_scroll->m_contentLayer->addChild(emptyLabel);
    return;
  }

  for (int i = 0; i < static_cast<int>(m_profiles.size()); ++i)
  {
    auto cell = createProfileCell(
        m_profiles[i],
        i,
        m_scroll->getContentWidth());

    if (cell)
      m_scroll->m_contentLayer->addChild(cell);
  }

  m_scroll->m_contentLayer->updateLayout();
  m_scroll->scrollToTop();
}

CCNode *WhiteListExport::createProfileCell(
    Profile const &profile,
    int index,
    float width)
{
  float const cellHeight = 24.f;

  auto cell = CCLayer::create();
  cell->setContentSize({width, cellHeight});
  cell->ignoreAnchorPointForPosition(false);

  // ! --- Background --- !
  auto cellBg = CCScale9Sprite::create("range-default-bg.png"_spr);
  cellBg->setContentSize({width - 4.f, cellHeight - 2.f});
  cellBg->setColor({17, 16, 16});
  cellBg->setOpacity(255 * .3f);
  cellBg->ignoreAnchorPointForPosition(true);
  cellBg->setPosition({2.f, 1.f});
  cell->addChild(cellBg);

  // ! --- Checkbox --- !
  auto checkboxMenu = CCMenu::create();
  checkboxMenu->setContentSize({22.f, cellHeight});
  checkboxMenu->setAnchorPoint({0.f, .5f});
  checkboxMenu->ignoreAnchorPointForPosition(false);
  checkboxMenu->setPosition({2.f, cellHeight / 2.f});

  auto checkbox = CCMenuItemToggler::createWithStandardSprites(
      this, menu_selector(WhiteListExport::onToggleProfile), 1.f);

  checkbox->setScale(.42f);

  checkbox->setPosition({
      checkboxMenu->getContentWidth() / 2.f,
      checkboxMenu->getContentHeight() / 2.f,
  });

  checkbox->setTag(index);

  checkbox->toggle(
      index < static_cast<int>(m_selected.size())
          ? m_selected[index]
          : true);

  checkboxMenu->addChild(checkbox);
  cell->addChild(checkboxMenu);

  m_profileCheckboxes.push_back(checkbox);

  // ! --- Name --- !
  bool streamerModEnabled =
      Mod::get()->getSettingValue<bool>("enable-streamer-mode");

  std::string profileName = profile.profileName;

  if (streamerModEnabled && !profileName.empty())
    profileName = profileName.substr(0, 1) + "...";

  if (profileName.length() > 26)
    profileName = profileName.substr(0, 26) + "...";

  auto nameLabel = CCLabelBMFont::create(
      profileName.c_str(), "bigFont.fnt");

  nameLabel->setScale(.32f);
  nameLabel->setAnchorPoint({0.f, .5f});
  nameLabel->setPosition({26.f, cellHeight * .66f});
  cell->addChild(nameLabel);

  // ! --- Stats --- !
  Profile statsProfile = profile;
  auto info = getMetaInfoFromStages(statsProfile.data.stages);

  int completedStages = std::min(
      std::max(info.completed, 0),
      info.total);

  std::string statsText =
      info.currentStage
          ? fmt::format(
                "Stages {}/{} Ranges {}/{}",
                completedStages + 1,
                info.total,
                info.currStageCompletedRanges,
                info.currStageTotalRanges)
          : fmt::format(
                "Stages {}/{} All ranges complete",
                completedStages,
                info.total);

  auto statsLabel = CCLabelBMFont::create(
      statsText.c_str(), "bigFont.fnt");

  statsLabel->setScale(.24f);
  statsLabel->setOpacity(255 * .55f);
  statsLabel->setAnchorPoint({0.f, .5f});
  statsLabel->setPosition({26.f, cellHeight * .28f});
  cell->addChild(statsLabel);

  return cell;
}

void WhiteListExport::createOptionCheckboxes()
{
  float const optionsWidth = m_size.width - 24.f;
  float const columnWidth = (optionsWidth - 8.f) / 2.f;
  float const rightX = 12.f + columnWidth + 8.f;

  auto secretsMenu = createOptionCheckbox(
      {columnWidth, 16.f},
      "Include secret integration data",
      m_includeSecrets,
      menu_selector(WhiteListExport::onToggleSecrets));

  secretsMenu->setPosition({12.f, 58.f});
  m_mainLayer->addChild(secretsMenu);

  auto progressionMenu = createOptionCheckbox(
      {columnWidth, 16.f},
      "Stage progression and completion",
      m_includeProgression,
      menu_selector(WhiteListExport::onToggleProgression));

  progressionMenu->setPosition({rightX, 58.f});
  m_mainLayer->addChild(progressionMenu);

  auto attemptsMenu = createOptionCheckbox(
      {columnWidth, 16.f},
      "Attempts, playtime and best runs",
      m_includeAttempts,
      menu_selector(WhiteListExport::onToggleAttempts));

  attemptsMenu->setPosition({12.f, 76.f});
  m_mainLayer->addChild(attemptsMenu);

  auto notesMenu = createOptionCheckbox(
      {columnWidth, 16.f},
      "Notes",
      m_includeNotes,
      menu_selector(WhiteListExport::onToggleNotes));

  notesMenu->setPosition({rightX, 76.f});
  m_mainLayer->addChild(notesMenu);
}

CCMenu *WhiteListExport::createOptionCheckbox(
    CCSize const &size,
    std::string const &label,
    bool checked,
    cocos2d::SEL_MenuHandler selector)
{
  auto checkbox = CCMenuItemToggler::createWithStandardSprites(
      this, selector, 1.f);

  checkbox->setScale(.4f);
  checkbox->ignoreAnchorPointForPosition(true);
  checkbox->setContentSize({32.f, 32.f});
  checkbox->toggle(checked);

  auto menu = CCMenu::createWithItem(checkbox);
  menu->setAnchorPoint({0.f, .5f});
  menu->setContentSize(size);
  menu->ignoreAnchorPointForPosition(false);

  menu->setLayout(
      RowLayout::create()
          ->setGap(4.f)
          ->setAutoScale(false)
          ->setAxisAlignment(AxisAlignment::Start));

  auto labelNode = CCLabelBMFont::create(
      label.c_str(), "bigFont.fnt");

  float availableWidth =
      size.width -
      checkbox->getContentWidth() * checkbox->getScale() -
      6.f;

  labelNode->setScale(
      std::min(
          .3f,
          availableWidth / labelNode->getContentWidth()));

  labelNode->setAnchorPoint({0.f, .5f});

  menu->addChild(labelNode);
  menu->updateLayout();

  return menu;
}

void WhiteListExport::createWarning()
{
  float const width = m_size.width - 40.f;
  float const scale = .34f;

  auto warning = Label::create(
      "This may include Discord webhook credentials. "
      "Only use this for a private backup.",
      "chatFont.fnt");

  warning->setOpacity(255);
  warning->setColor({255, 130, 90});
  warning->setScale(scale);
  warning->setAlignment(Label::Alignment::Center);

  warning->setMaxWidth(width / scale);
  warning->setBreakWords(true);

  warning->setAnchorPoint({.5f, 0.f});
  warning->setPosition({m_size.width / 2.f, 38.f});
  warning->setVisible(m_includeSecrets);

  m_mainLayer->addChild(warning);
  m_warningLabel = warning;
}

void WhiteListExport::createBottomButtons()
{
  auto menu = CCMenu::create();
  menu->setAnchorPoint({.5f, 0.f});
  menu->setPosition({m_size.width / 2.f, 10.f});

  auto cancelSpr = ButtonSprite::create(
      "Cancel", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.f, .8f);
  auto cancelBtn = CCMenuItemSpriteExtra::create(
      cancelSpr, this, menu_selector(WhiteListExport::onCancel));
  cancelBtn->setScale(.8f);
  cancelBtn->ignoreAnchorPointForPosition(true);

  auto exportSpr = ButtonSprite::create(
      "Export", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.f, .8f);
  auto exportBtn = CCMenuItemSpriteExtra::create(
      exportSpr, this, menu_selector(WhiteListExport::onExport));
  exportBtn->setScale(.8f);
  exportBtn->ignoreAnchorPointForPosition(true);

  menu->addChild(cancelBtn);
  menu->addChild(exportBtn);

  menu->setLayout(
      RowLayout::create()
          ->setGap(8.f)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::Center)
          ->setCrossAxisAlignment(AxisAlignment::Center));

  m_mainLayer->addChild(menu);
  menu->updateLayout();
}

// ! --- Handlers --- !

void WhiteListExport::onToggleProfile(CCObject *sender)
{
  auto checkbox = static_cast<CCMenuItemToggler *>(sender);
  int index = checkbox->getTag();

  if (index < 0 || index >= static_cast<int>(m_selected.size()))
    return;

  m_selected[index] = !m_selected[index];
}

void WhiteListExport::onSelectAll(CCObject *)
{
  refreshProfileCheckboxes(true);
}

void WhiteListExport::onDeselectAll(CCObject *)
{
  refreshProfileCheckboxes(false);
}

void WhiteListExport::refreshProfileCheckboxes(bool selected)
{
  m_selected.assign(m_selected.size(), selected);

  for (auto *checkbox : m_profileCheckboxes)
    if (checkbox)
      checkbox->toggle(selected);
}

void WhiteListExport::onToggleSecrets(CCObject *)
{
  m_includeSecrets = !m_includeSecrets;

  if (m_warningLabel)
    m_warningLabel->setVisible(m_includeSecrets);
}

void WhiteListExport::onToggleProgression(CCObject *)
{
  m_includeProgression = !m_includeProgression;
}

void WhiteListExport::onToggleAttempts(CCObject *)
{
  m_includeAttempts = !m_includeAttempts;
}

void WhiteListExport::onToggleNotes(CCObject *)
{
  m_includeNotes = !m_includeNotes;
}

void WhiteListExport::onCancel(CCObject *sender)
{
  this->onClose(sender);
}

void WhiteListExport::onExport(CCObject *sender)
{
  std::vector<Profile> filtered;
  filtered.reserve(m_profiles.size());

  for (size_t i = 0; i < m_profiles.size(); ++i)
  {
    if (i >= m_selected.size() || !m_selected[i])
      continue;

    filtered.push_back(
        filterProfileForExport(
            m_profiles[i],
            m_includeSecrets,
            m_includeProgression,
            m_includeAttempts,
            m_includeNotes));
  }

  if (filtered.empty())
  {
    FLAlertLayer::create(
        "Export Error",
        "Select at least one profile to export.",
        "OK")
        ->show();

    return;
  }

  auto const backupDir =
      Mod::get()->getSaveDir() / "backups";

  auto const backupFile =
      backupDir / backup::generateBackupFilename();

  auto directoryResult =
      geode::utils::file::createDirectory(backupDir);

  if (directoryResult.isErr())
  {
    log::error(
        "Unable to create backup directory: {}",
        directoryResult.unwrapErr());

    return;
  }

  matjson::Value json = filtered;

  auto writeResult = geode::utils::file::writeStringSafe(
      backupFile,
      json.dump(matjson::NO_INDENTATION));

  if (writeResult.isErr())
  {
    log::error(
        "Unable to save backup: {}",
        writeResult.unwrapErr());

    return;
  }

  geode::utils::file::openFolder(backupFile);
  this->onClose(sender);
}
