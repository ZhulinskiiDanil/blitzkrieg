#include "RangeLabel.hpp"

RangeLabel *RangeLabel::create(
    Range *range,
    const std::string &font)
{
  RangeLabel *ret = new RangeLabel();
  ret->enabled = range->checked;

  if (ret && ret->init(range, font))
  {
    ret->autorelease();
    return ret;
  }
  delete ret;
  return nullptr;
}

bool RangeLabel::init(Range *range, const std::string &font)
{
  if (!CCNode::init())
    return false;

  std::string rangeText = geode::utils::numToString(range->from) + "-" + geode::utils::numToString(range->to) +
                          " (" + geode::utils::numToString(std::abs(range->to - range->from)) + "%)";
  std::string rangeNote = range->note;

  m_label = CCLabelBMFont::create(rangeText.c_str(), font.c_str());
  m_label->setScale(0.4f);
  m_label->setAnchorPoint({0, 0.5f});

  if (!rangeNote.empty())
  {
    m_label->setPositionY(5.f);

    m_noteLabel = CCLabelBMFont::create(rangeNote.c_str(), "gjFont17.fnt");
    m_noteLabel->setScale(0.25f);
    m_noteLabel->setAnchorPoint({0, 0.5f});
    m_noteLabel->setPositionY(-7.f);
    m_noteLabel->setOpacity(255 * 0.7f);
  }

  updateColor();

  if (m_noteLabel)
    this->addChild(m_noteLabel);

  this->addChild(m_label);
  return true;
}

void RangeLabel::setRange(int from, int to)
{
  if (m_label)
  {
    std::string rangeText = geode::utils::numToString(from) + "-" + geode::utils::numToString(to);
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