#include "index.hpp"

CreateProfilePopup *CreateProfilePopup::create(GJGameLevel *level)
{
  CreateProfilePopup *ret = new CreateProfilePopup();

  // ret->initAnchored(340, 248, level, "GJ_square01_custom.png"_spr)
  if (ret->init(level))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool CreateProfilePopup::init(GJGameLevel *level)
{
  if (!Popup::init(320, 280, "GJ_square01_custom.png"_spr))
  {
    return false;
  }

  m_level = level;
  setTitle("Create Profile");

  findStartPoses();

  bool hasStartPoses = m_2_1_percentages.size() > 0;

  m_bottomButtonMenu = CCMenu::create();
  m_bottomButtonMenu->setAnchorPoint({.0f, 0.f});
  m_bottomButtonMenu->setPosition({10.f, 10.f});

  // ! --- Cancel Button --- !
  m_button2 = ButtonSprite::create("Cancel", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.0f, .8f);
  m_button2->setScale(.8f);

  auto btnCancel = CCMenuItemSpriteExtra::create(
      m_button2,
      this,
      menu_selector(CreateProfilePopup::onBtn2));

  m_bottomButtonMenu->addChild(btnCancel);
  btnCancel->ignoreAnchorPointForPosition(true);

  // ! --- Button Menu --- !
  m_bottomButtonMenu->setLayout(
      RowLayout::create()
          ->setGap(5.f)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::Center)
          ->setCrossAxisAlignment(AxisAlignment::Center));

  // ! --- Start Positions Validation --- !
  if (!hasStartPoses)
  {
    auto emptyLabel = CCLabelBMFont::create(
        "No start positions found in this level",
        "bigFont.fnt");
    emptyLabel->setScale(.4f);
    emptyLabel->setAnchorPoint({.5f, .5f});
    emptyLabel->setPosition(m_size / 2);
    emptyLabel->setOpacity(255 * .5f);

    m_mainLayer->addChild(emptyLabel);

    return true;
  }

  // ! --- Create Profile Button --- !
  m_button1 = ButtonSprite::create("Create Profile", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.0f, .8f);
  m_button1->setScale(.8f);

  auto btnCreateProfile = CCMenuItemSpriteExtra::create(
      m_button1,
      this,
      menu_selector(CreateProfilePopup::onCreateProfile));

  m_bottomButtonMenu->addChild(btnCreateProfile);
  btnCreateProfile->ignoreAnchorPointForPosition(true);

  // ! --- Input --- !
  m_input = TextInput::create(m_size.width - 20.f, "Profile name", "bigFont.fnt");
  m_input->getInputNode()->setMaxLabelLength(32);
  m_input->setAnchorPoint({0.f, 1.f});
  m_input->setPosition(10.f, m_size.height - 40.f);
  m_input->setString(m_level->m_levelName, false);
  m_input->setMaxCharCount(32);

  // ! --- Lists Container --- !
  m_lists = CCLayer::create();
  m_lists->setAnchorPoint({.5f, 1.f});
  m_lists->setPosition(m_size.width / 2, m_input->getPositionY() - m_input->getContentHeight() - 8.f);
  m_lists->setLayout(
      ColumnLayout::create()
          ->setGap(4.f)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::Start)
          ->setCrossAxisAlignment(AxisAlignment::Start));

  // ! --- Draw Lists --- !
  for (size_t i = 0; i < 2; i++)
  {
    auto leftList = CCLayer::create();
    auto rightList = CCLayer::create();
    auto percentages = i == 0 ? m_2_1_percentages : m_2_2_percentages;
    // 5 cell will be in center with "..." text
    std::vector<float> leftPercentages(percentages.begin(), percentages.begin() + 5);
    std::vector<float> rightPercentages(percentages.end() - 4, percentages.end());
    std::vector<float> combined;
    combined.reserve(leftPercentages.size() + rightPercentages.size());
    combined.insert(combined.end(), leftPercentages.begin(), leftPercentages.end());
    combined.insert(combined.end(), rightPercentages.begin(), rightPercentages.end());

    bool useCompactLayout = percentages.size() > 9;

    const auto listCell = CCLayer::create();
    listCell->setContentSize({m_size.width - 20.f - 8.f, 20.f});
    listCell->setAnchorPoint({0.f, .5f});

    leftList->setContentSize({listCell->getContentWidth() - 4.f, listCell->getContentHeight() - 4.f});
    leftList->setPosition({2.f, 2.f});
    leftList->setLayout(
        RowLayout::create()
            ->setGap(2.f)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisAlignment(AxisAlignment::Center));

    rightList->setContentSize({listCell->getContentWidth() - 4.f, listCell->getContentHeight() - 4.f});
    rightList->setPosition({2.f, 2.f});
    rightList->setLayout(
        RowLayout::create()
            ->setGap(2.f)
            ->setAxisAlignment(AxisAlignment::End)
            ->setCrossAxisAlignment(AxisAlignment::Center));
    rightList->setVisible(useCompactLayout);

    const auto targetPercentages = useCompactLayout ? combined : percentages;
    for (size_t j = 0; j < targetPercentages.size(); j++)
    {
      // ! --- Cell --- !
      const auto cell = CCLayer::create();
      cell->setContentSize({32.f, leftList->getContentHeight()});
      cell->ignoreAnchorPointForPosition(false);

      if (useCompactLayout && j == 4)
        cell->setPosition(listCell->getContentSize() / 2);
      else
        cell->setPosition({0, 0});

      // ! --- Background --- !
      auto background = CCScale9Sprite::create("range-default-bg.png"_spr);
      background->setContentSize(cell->getContentSize());
      background->setColor({17, 16, 16});
      background->setOpacity(255 * 0.3f);
      background->ignoreAnchorPointForPosition(true);

      // ! --- Text --- !
      std::string text = j == 4 && useCompactLayout ? "..." : fmt::format("{:.2f}", targetPercentages[j]);
      auto rangeLabel = CCLabelBMFont::create(text.c_str(), "gjFont17.fnt");
      rangeLabel->setScale(.25f);
      rangeLabel->setAnchorPoint({.5f, .5f});
      rangeLabel->setPosition({background->getContentWidth() / 2 + background->getPositionX(),
                               background->getContentHeight() / 2 + background->getPositionY()});

      if (j != 4)
        cell->addChild(background);
      cell->addChild(rangeLabel);

      if (!useCompactLayout || (useCompactLayout && j < 4))
        leftList->addChild(cell);
      else if (j != 4)
        rightList->addChild(cell);
      else
        listCell->addChild(cell);
    }

    listCell->addChild(leftList);
    listCell->addChild(rightList);

    m_lists->addChild(listCell);
    leftList->updateLayout();
    rightList->updateLayout();
    leftList->ignoreAnchorPointForPosition(true);
    rightList->ignoreAnchorPointForPosition(true);
  }

  // ! --- Border --- !
  int currCellIndex = m_2_1_percentagesChecked ? 1 : 0;
  auto *percentagesCell = CCArrayExt<CCLayer *>(m_lists->getChildren())[currCellIndex];

  m_border = ListBorders::create();
  m_border->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f);

  if (percentagesCell)
  {
    m_border->setContentSize({percentagesCell->getContentWidth() + 2.f,
                              percentagesCell->getContentHeight()});
    m_border->setPosition({m_size.width / 2,
                           m_lists->getPositionY() - percentagesCell->getContentHeight() / 2 - (percentagesCell->getContentHeight() + 4) * currCellIndex});
  }

  for (auto child : CCArrayExt<CCNodeRGBA *>(m_border->getChildren()))
  {
    child->setColor(ccc3(75, 210, 75));
    child->setOpacity(m_border->isVisible() ? 255 : 0);
  }

  m_mainLayer->addChild(m_border);
  m_mainLayer->addChild(m_input);
  m_mainLayer->addChild(m_lists);
  m_mainLayer->addChild(m_bottomButtonMenu);
  m_lists->updateLayout();
  m_bottomButtonMenu->updateLayout();

  m_checkboxesMenu = CCMenu::create();
  m_checkboxesMenu->setContentSize({m_mainLayer->getContentWidth() - 28.0f, 32.0f});
  m_checkboxesMenu->setAnchorPoint({0.0f, 0.0f});
  m_checkboxesMenu->setPosition({12.0f, m_button1->getPositionY() + m_button1->getContentHeight() / 2.0f + 13.0f});
  m_checkboxesMenu->setLayout(RowLayout::create()
                                  ->setAxisReverse(true)
                                  ->setGap(5.0f)
                                  ->setAutoScale(false));

  // ! --- Percentages Checkbox --- !
  m_percentagesCheckbox = CCMenuItemToggler::createWithStandardSprites(
      this, menu_selector(CreateProfilePopup::onTogglePercentages), 1.0f);
  m_percentagesCheckbox->setScale(0.5f);
  m_percentagesCheckbox->ignoreAnchorPointForPosition(true);
  m_percentagesCheckbox->setContentSize({32.f, 32.f});
  m_percentagesCheckbox->toggle(m_2_1_percentagesChecked);

  auto percentagesCheckboxMenu = CCMenu::createWithItem(m_percentagesCheckbox);
  percentagesCheckboxMenu->setAnchorPoint({.0f, .5f});
  percentagesCheckboxMenu->setContentSize({m_checkboxesMenu->getContentWidth() / 3.0f - (10.0f / 3.0f), m_checkboxesMenu->getContentHeight()});
  percentagesCheckboxMenu->ignoreAnchorPointForPosition(false);
  percentagesCheckboxMenu->setLayout(RowLayout::create()
                                         ->setGap(2.0f)
                                         ->setAutoScale(false));

  // ! --- Checkbox Text --- !
  auto percentagesCheckboxLabel = CCLabelBMFont::create("2.1 percentages", "bigFont.fnt");
  percentagesCheckboxLabel->setScale((percentagesCheckboxMenu->getContentWidth() - m_percentagesCheckbox->getContentWidth() / 2.0f) / percentagesCheckboxLabel->getContentWidth() * 0.95f);
  percentagesCheckboxLabel->setAnchorPoint({0, .5f});

  percentagesCheckboxMenu->addChild(percentagesCheckboxLabel);
  m_checkboxesMenu->addChild(percentagesCheckboxMenu);
  percentagesCheckboxMenu->updateLayout();

  // ! --- Use Checkbox --- !
  m_useCheckbox = CCMenuItemToggler::createWithStandardSprites(
      this, menu_selector(CreateProfilePopup::onToggleUse), 1.0f);
  m_useCheckbox->setScale(0.5f);
  m_useCheckbox->ignoreAnchorPointForPosition(true);
  m_useCheckbox->setContentSize({32.f, 32.f});
  m_useCheckbox->toggle(m_useChecked);

  auto useCheckboxMenu = CCMenu::createWithItem(m_useCheckbox);
  useCheckboxMenu->setAnchorPoint({.0f, .5f});
  useCheckboxMenu->setContentSize({m_checkboxesMenu->getContentWidth() / 3.0f - (10.0f / 3.0f), m_checkboxesMenu->getContentHeight()});
  useCheckboxMenu->ignoreAnchorPointForPosition(false);
  useCheckboxMenu->setLayout(RowLayout::create()
                                 ->setGap(2.0f)
                                 ->setAutoScale(false));

  // ! --- Checkbox Text --- !
  auto useCheckboxLabel = CCLabelBMFont::create("Bind profile", "bigFont.fnt");
  useCheckboxLabel->setScale((useCheckboxMenu->getContentWidth() - m_useCheckbox->getContentWidth() / 2.0f) / useCheckboxLabel->getContentWidth() * 0.95f);
  useCheckboxLabel->setAnchorPoint({0, .5f});

  useCheckboxMenu->addChild(useCheckboxLabel);
  m_checkboxesMenu->addChild(useCheckboxMenu);
  useCheckboxMenu->updateLayout();

  // ! --- Pin Checkbox --- !
  m_pinCheckbox = CCMenuItemToggler::createWithStandardSprites(
      this, menu_selector(CreateProfilePopup::onTogglePin), 1.0f);
  m_pinCheckbox->setScale(0.5f);
  m_pinCheckbox->ignoreAnchorPointForPosition(true);
  m_pinCheckbox->setContentSize({32.f, 32.f});
  m_pinCheckbox->toggle(m_pinChecked);

  auto pinCheckboxMenu = CCMenu::createWithItem(m_pinCheckbox);
  pinCheckboxMenu->setAnchorPoint({.0f, .5f});
  pinCheckboxMenu->setContentSize({m_checkboxesMenu->getContentWidth() / 3.0f - (10.0f / 3.0f), m_checkboxesMenu->getContentHeight()});
  pinCheckboxMenu->ignoreAnchorPointForPosition(false);
  pinCheckboxMenu->setLayout(RowLayout::create()
                                 ->setGap(2.0f)
                                 ->setAutoScale(false));

  // ! --- Checkbox Text --- !
  auto pinCheckboxLabel = CCLabelBMFont::create("Pin profile", "bigFont.fnt");
  pinCheckboxLabel->setScale((pinCheckboxMenu->getContentWidth() - m_pinCheckbox->getContentWidth() / 2.0f) / pinCheckboxLabel->getContentWidth() * 0.95f);
  pinCheckboxLabel->setAnchorPoint({0, .5f});

  pinCheckboxMenu->addChild(pinCheckboxLabel);
  m_checkboxesMenu->addChild(pinCheckboxMenu);
  pinCheckboxMenu->updateLayout();

  m_checkboxesMenu->updateLayout();
  m_mainLayer->addChild(m_checkboxesMenu);

  // ! --- ToggablePercentagesList --- !
  auto percentages = m_2_1_percentagesChecked ? m_2_1_percentages : m_2_2_percentages;
  m_percentagesList = ToggablePercentagesList::create(
      {m_mainLayer->getContentWidth() - 24.0f, 80.0f},
      percentages);
  m_percentagesList->setPosition({m_mainLayer->getContentWidth() / 2.0f, m_lists->getPositionY() - m_lists->getContentHeight() - 10.0f});
  m_mainLayer->addChild(m_percentagesList);

  // ! --- Other --- !
  this->setKeypadEnabled(true);
  this->setTouchEnabled(true);

  return true;
}

void CreateProfilePopup::findStartPoses()
{
  auto startposes = findStartposesFromCurrentLevel();
  m_2_1_percentages = startposes.percentages_2_1;
  m_2_2_percentages = startposes.percentages_2_2;
}

void CreateProfilePopup::onCreateProfile(CCObject *sender)
{
  std::string levelName = m_input->getString();

  if (levelName.length() == 0)
  {
    const auto alert = FLAlertLayer::create(
        "Profile Name Required",
        "Please enter a profile name before continuing.",
        "OK");
    alert->show();

    return;
  }

  auto profile1 = generateProfile(levelName, m_2_1_percentagesChecked ? m_2_1_percentages : m_2_2_percentages).as<Profile>().unwrap();
  auto profile2 = generateProfile(levelName, m_percentagesList->getEnabledStartposes()).as<Profile>().unwrap();

  auto profile = mergeProfiles(profile1, profile2, false);
  GlobalStore::get()->updateProfile(profile);

  if (m_useChecked)
    linkProfileWithLevel(profile, m_level);
  if (m_pinChecked)
    GlobalStore::get()->pinProfileById(profile.id, true);

  this->onClose(sender);
  ProfilesChangedEvent().send();
}

void CreateProfilePopup::onTogglePercentages(CCObject *sender)
{
  if (m_percentagesCheckbox && m_border && m_lists)
  {
    m_border->stopAllActions();
    m_2_1_percentagesChecked = !m_2_1_percentagesChecked;

    int currCellIndex = m_2_1_percentagesChecked ? 1 : 0;
    auto *percentagesCell = CCArrayExt<CCLayer *>(m_lists->getChildren())[currCellIndex];

    if (percentagesCell)
    {
      auto moveTo = CCMoveTo::create(
          0.15f,
          CCPoint(
              m_size.width / 2,
              m_lists->getPositionY() - percentagesCell->getContentHeight() / 2 -
                  (percentagesCell->getContentHeight() + 4) * currCellIndex));

      auto easeMove = CCEaseInOut::create(moveTo, 2.0f);
      m_border->runAction(easeMove);

      for (auto child : CCArrayExt<CCNodeRGBA *>(m_border->getChildren()))
      {
        auto tintTo = m_2_1_percentagesChecked
                          ? CCTintTo::create(0.15f, 75, 210, 75)
                          : CCTintTo::create(0.15f, 255, 170, 0);

        auto easeTint = CCEaseInOut::create(tintTo, 2.0f);
        child->runAction(easeTint);
      }

      m_percentagesList->setStartposes(
          m_2_1_percentagesChecked ? m_2_1_percentages : m_2_2_percentages);
    }
  }
}

void CreateProfilePopup::onToggleUse(CCObject *sender)
{
  if (m_useCheckbox)
    m_useChecked = !m_useChecked;
}

void CreateProfilePopup::onTogglePin(CCObject *sender)
{
  if (m_pinCheckbox)
    m_pinChecked = !m_pinChecked;
}