#include "RangeLabel.hpp"

RangeLabel *RangeLabel::create(int from, int to, bool enabled, const std::string &font)
{
  RangeLabel *ret = new RangeLabel();
  ret->enabled = enabled;

  if (ret && ret->init(from, to, font))
  {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

bool RangeLabel::init(int from, int to, const std::string &font)
{
  if (!CCNode::init())
    return false;

  std::string rangeText = std::to_string(from) + "-" + std::to_string(to) +
                          " (" + std::to_string(std::abs(to - from)) + "%)";
  m_label = CCLabelBMFont::create(rangeText.c_str(), font.c_str());

  if (!m_label)
    return false;

  m_label->setScale(0.5f);
  m_label->setAnchorPoint({0, 0.5f});

  updateColor();

  this->addChild(m_label);
  return true;
}

void RangeLabel::setRange(int from, int to)
{
  if (m_label)
  {
    std::string rangeText = std::to_string(from) + "-" + std::to_string(to);
    m_label->setString(rangeText.c_str());
  }
}

void RangeLabel::setEnabled(bool value)
{
  enabled = value;
  updateColor();
}

void RangeLabel::updateColor()
{
  if (!enabled)
    m_label->setColor(disabledColor);
  else
    m_label->setColor(enabledColor);
}