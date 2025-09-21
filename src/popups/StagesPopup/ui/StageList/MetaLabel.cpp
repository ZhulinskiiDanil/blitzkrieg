#include "MetaLabel.hpp"

MetaLabel *MetaLabel::create(float width, const std::string &label, const std::string &value)
{
  auto *ret = new MetaLabel();
  if (ret && ret->init(width, label, value))
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool MetaLabel::init(float width, const std::string &label, const std::string &value)
{
  if (!CCLayer::init())
    return false;

  this->setLayout(
      RowLayout::create()
          ->setGap(m_gap)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setAxisAlignment(AxisAlignment::Center)
          ->setCrossAxisLineAlignment(AxisAlignment::Start));

  m_label = Label::create(label.c_str(), "gjFont17.fnt", .25f);
  m_label->setAnchorPoint({0.f, 1.f});
  m_label->setPosition({0.f, 0.f});
  this->addChild(m_label, 1);
  this->updateLayout();

  m_value = Label::create(value, "gjFont17.fnt", .25f);
  m_value->setAnchorPoint({0.f, 1.f});
  m_value->setPosition({m_label->getContentSize().width + 10.f, 0.f});
  this->addChild(m_value, 3);
  this->updateLayout();

  auto line = CCLayerColor::create({255, 255, 255, 10},
                                   width - m_label->getContentWidth() - m_value->getContentWidth() - m_gap * 2,
                                   1.f);
  this->addChild(line, 2);
  this->updateLayout();

  return true;
}
