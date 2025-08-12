#include "index.hpp"

// StagesPopup* StagesPopup::create(CreateFolderDelegate* delegate) {
StagesPopup *StagesPopup::create()
{
  StagesPopup *ret = new StagesPopup();

  // Запоминаем делегата
  // ret->m_delegate = delegate;

  // Если получается инициализироват модалку, то вызываем autorelease
  // Чтобы она сама очистилась из памяти когда не будет использоваться
  if (ret->initAnchored(300, 135))
  {
    ret->autorelease();
    return ret;
  }

  // Не удалось инициализировать, поэтому удаляем
  delete ret;
  return nullptr;
}

bool StagesPopup::setup()
{
  setTitle("Create Folder");

  m_input = TextInput::create(170, "Name", "bigFont.fnt");
  m_input->getInputNode()->setMaxLabelLength(60);
  m_input->setPosition({m_size.width / 2, 74});

  m_mainLayer->addChild(m_input);

  ButtonSprite *btnSpr = ButtonSprite::create("Create");
  btnSpr->setScale(0.8f);

  CCMenuItemSpriteExtra *btn = CCMenuItemSpriteExtra::create(
      btnSpr,
      this,
      menu_selector(StagesPopup::onCreate));
  btn->setPosition(m_size / 2 - ccp(0, 40));

  m_buttonMenu->addChild(btn);

  return true;
}

void StagesPopup::onCreate(CCObject *)
{
  std::string name = m_input->getString();

  if (name.empty())
    return FLAlertLayer::create("Error", "Folder name can't be <cl>empty</c>.", "Ok")->show();

  onClose(nullptr);

  // Вызываем onFolderCreate у делегата
  // m_delegate->onFolderCreate(name);
}