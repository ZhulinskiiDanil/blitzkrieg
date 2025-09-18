#include "index.hpp"

CreateProfilePopup *CreateProfilePopup::create(GJGameLevel *level)
{
  CreateProfilePopup *ret = new CreateProfilePopup();

  if (ret->initAnchored(340, 200, level, "GJ_square01_custom.png"_spr))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool CreateProfilePopup::setup(GJGameLevel *level)
{
  m_level = level;
  setTitle("Create Profile");

  findStartPoses();

  auto hasStartPoses = m_2_1_percentages.size() > 0;

  m_buttonMenu = CCMenu::create();
  m_buttonMenu->setAnchorPoint({.0f, 0.f});
  m_buttonMenu->setPosition({10.f, 10.f});

  // ! --- Cancel Button --- !
  m_button2 = ButtonSprite::create("Cancel", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.0f, .8f);
  m_button2->setScale(.8f);

  auto btnOK = CCMenuItemSpriteExtra::create(
      m_button2,
      this,
      menu_selector(CreateProfilePopup::onBtn2));

  m_buttonMenu->addChild(btnOK);
  btnOK->ignoreAnchorPointForPosition(true);

  // ! --- Button Menu --- !
  m_buttonMenu->setLayout(
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

  m_buttonMenu->addChild(btnCreateProfile);
  btnCreateProfile->ignoreAnchorPointForPosition(true);

  // ! --- Input --- !
  m_input = TextInput::create(m_size.width - 20.f, "Profile name", "bigFont.fnt");
  m_input->getInputNode()->setMaxLabelLength(32);
  m_input->setAnchorPoint({0.f, 1.f});
  m_input->setPosition(10.f, m_size.height - 40.f);

  // ! --- Lists Container --- !
  const auto lists = CCLayer::create();
  lists->setAnchorPoint({.5f, 1.f});
  lists->setPosition(m_size.width / 2, m_input->getPositionY() - m_input->getContentHeight() - 10.f);
  lists->setLayout(
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

      cell->addChild(background);
      cell->addChild(rangeLabel);

      if (!useCompactLayout || (useCompactLayout && j < 4))
        leftList->addChild(cell);
      else if (j != 4)
        rightList->addChild(cell);
      else
        listCell->addChild(cell);
    }

    // ! --- Borders --- !
    auto borders = ListBorders::create();
    borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f);

    if (i == 0)
    {
      m_borders1 = borders;
      borders->setVisible(m_checked);
    }
    else
    {
      m_borders2 = borders;
      borders->setVisible(!m_checked);
    }

    for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
      child->setColor(ccc3(75, 210, 75));

    listCell->addChild(leftList);
    listCell->addChild(rightList);
    listCell->addChild(borders);

    lists->addChild(listCell);
    leftList->updateLayout();
    rightList->updateLayout();
    leftList->ignoreAnchorPointForPosition(true);
    rightList->ignoreAnchorPointForPosition(true);

    borders->setContentSize({listCell->getContentWidth() + 2.f,
                             listCell->getContentHeight()});
    borders->setPosition({(m_size.width - 20.f) / 2 - 4.f, listCell->getContentHeight() / 2});
  }

  // ! --- Checkbox --- !
  auto toggleOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

  m_mainLayer->addChild(m_input);
  m_mainLayer->addChild(lists);
  lists->updateLayout();
  m_mainLayer->addChild(m_buttonMenu);
  m_buttonMenu->updateLayout();

  m_checkbox = CCMenuItemToggler::create(toggleOn, toggleOff, this, menu_selector(CreateProfilePopup::onToggle));
  m_checkbox->ignoreAnchorPointForPosition(true);
  m_checkbox->setContentSize({32.f, 32.f});
  m_checkbox->toggle(!m_checked);

  auto menu = CCMenu::createWithItem(m_checkbox);
  menu->setScale(.6f);
  menu->setAnchorPoint({.0f, .5f});
  menu->setContentSize(m_checkbox->getContentSize());
  menu->ignoreAnchorPointForPosition(false);
  menu->setPosition({10.f + 2.f,
                     lists->getPositionY() - lists->getContentHeight() - 8.f - 12.f});
  menu->setLayout(RowLayout::create());

  m_mainLayer->addChild(menu);
  menu->updateLayout();

  // ! --- Checkbox Text --- !
  auto noteLabel = CCLabelBMFont::create("Old 2.1 percentages", "bigFont.fnt");
  noteLabel->setScale(0.4f);
  noteLabel->setAnchorPoint({0, .5f});
  noteLabel->setPosition({menu->getPositionX() + menu->getContentWidth() / 2 + 8.f,
                          menu->getPositionY()});

  m_mainLayer->addChild(noteLabel);

  this->setKeypadEnabled(true);
  this->setTouchEnabled(true);

  return true;
}

void CreateProfilePopup::findStartPoses()
{
  for (auto child : CCArrayExt<StartPosObject *>(PlayLayer::get()->m_objects))
  {
    if (auto startPos = typeinfo_cast<StartPosObject *>(child))
    {
      const float levelLength = PlayLayer::get()->m_levelLength;
      const float levelTime = PlayLayer::get()->timeForPos({levelLength, 0.f}, 0.f, 0.f, true, 0.f);

      const float startPosX = startPos->getPositionX();
      const float startPosPercentByPosX = (startPosX / levelLength) * 100.f;

      const float startPosTime = PlayLayer::get()->timeForPos({startPosX, 0.f}, 0.f, 0.f, true, 0.f);
      const float startPosPercentByTime = (startPosTime / levelTime) * 100.f;

      m_2_1_percentages.push_back(startPosPercentByPosX);
      m_2_2_percentages.push_back(startPosPercentByTime);
    }
  }

  std::sort(m_2_1_percentages.begin(), m_2_1_percentages.end());
  std::sort(m_2_2_percentages.begin(), m_2_2_percentages.end());
}

void CreateProfilePopup::onCreateProfile(CCObject *sender)
{
  std::string levelName = m_input->getString();

  if (levelName.length() <= 0)
  {
    const auto alert = FLAlertLayer::create(
        "Profile Name Required",
        "Please enter a profile name before continuing.",
        "OK");
    alert->show();

    return;
  }

  if (levelName.length() > 32)
  {
    const auto alert = FLAlertLayer::create(
        "Invalid Profile Name",
        "Your profile name must not exceed 32 characters.",
        "OK");
    alert->show();

    return;
  }

  matjson::Value profile1 = generateProfile(levelName, m_2_1_percentages);
  matjson::Value profile2 = generateProfile(levelName, m_2_2_percentages);

  auto profile = m_checked ? profile1 : profile2;
  GlobalStore::get()->updateProfile(profile.as<Profile>().unwrap());

  this->onClose(sender);
  ProfilesChangedEvent().post();
}

void CreateProfilePopup::onToggle(CCObject *sender)
{
  if (m_checkbox)
  {
    m_checked = !m_checked;
    m_checkbox->toggle(m_checked);

    m_borders1->setVisible(m_checked);
    m_borders2->setVisible(!m_checked);
  }
}