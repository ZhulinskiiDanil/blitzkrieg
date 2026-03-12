#include "index.hpp"

EditStartPosPopup *EditStartPosPopup::create(float startpos, std::function<void(float, float)> callback)
{
  auto ret = new EditStartPosPopup();

  if (ret->init(startpos, callback))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool EditStartPosPopup::init(float startpos, std::function<void(float, float)> callback)
{
  if (!Popup::init(320, 116, "GJ_square01_custom.png"_spr))
    return false;

  m_oldStartpos = startpos;
  m_newStartpos = startpos;
  m_callback = callback;

  auto title = fmt::format("Edit Startpos {:.2f}%", m_newStartpos);
  setTitle(title.c_str());

  // Input
  m_startposInput = TextInput::create(m_size.width - 20.f, "0.00", "bigFont.fnt");
  m_startposInput->setFilter("0123456789.");
  m_startposInput->setAnchorPoint({0, 1});
  m_startposInput->setPosition(10.f, m_size.height - 30.f - 10.f);
  m_startposInput->setString(geode::utils::numToString(m_newStartpos));

  m_mainLayer->addChild(m_startposInput);

  // Buttons
  m_buttonMenu = CCMenu::create();
  m_buttonMenu->setPosition({10.f, 10.f});
  m_buttonMenu->setAnchorPoint({0, 0});

  auto save = CCMenuItemSpriteExtra::create(
      ButtonSprite::create("Save", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.0f, .8f),
      this,
      menu_selector(EditStartPosPopup::onSave));

  auto cancel = CCMenuItemSpriteExtra::create(
      ButtonSprite::create("Cancel", 0, 0, "goldFont.fnt", "GJ_button_01.png", 0.0f, .8f),
      this,
      menu_selector(EditStartPosPopup::onCancel));

  m_buttonMenu->addChild(save);
  m_buttonMenu->addChild(cancel);

  m_buttonMenu->setLayout(
      RowLayout::create()
          ->setGap(5.f)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::Center));

  m_mainLayer->addChild(m_buttonMenu);
  m_buttonMenu->updateLayout();

  return true;
}

void EditStartPosPopup::onCancel(CCObject *)
{
  this->onClose(nullptr);
}

void EditStartPosPopup::onSave(CCObject *)
{
  auto result = geode::utils::numFromString<float>(m_startposInput->getString()).unwrap();

  if (m_callback && result > 0.f && result < 100.f)
  {
    geode::createQuickPopup(
        "Expert Feature Warning",
        fmt::format(
            "This feature is intended for advanced users.\n\n"
            "Are you sure you want to change the start position from "
            "{:.2f}% to {:.2f}%?\n\n"
            "Incorrect values may break your startpos configuration.",
            m_oldStartpos,
            result),
        "Cancel",
        "Confirm",
        [this, result](auto, bool confirmed)
        {
          if (confirmed)
          {
            m_callback(m_oldStartpos, result);
            this->onClose(nullptr);
          }
        });
  }
  else
  {
    FLAlertLayer::create(
        "Save Error",
        "Percentages must be greater than 0 and less than 100",
        "OK")
        ->show();
  }
}