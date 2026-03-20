#include "./GraphPointDisplay.hpp"

GraphPointDisplay *GraphPointDisplay::create(std::string text)
{
  auto ret = new GraphPointDisplay();
  if (ret && ret->init(text))
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool GraphPointDisplay::init(std::string text)
{
  if (!CCLayer::init())
    return false;

  this->setContentSize({60, 20});

  m_label = Label::create(text);
  m_label->setPosition(this->getContentSize() / 2);
  m_label->setScale(.3f);
  m_label->setZOrder(1);

  this->addChild(m_label);

  auto rectNode = RectNode::create(
      this->getContentSize(),
      ccc4FFromccc4B({50, 50, 50, 255}),
      5);

  this->addChild(rectNode);

  return true;
}

void GraphPointDisplay::setText(const std::string &text)
{
  if (m_label)
    m_label->setText(text);
}