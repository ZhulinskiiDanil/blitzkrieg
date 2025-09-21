#include "Label.hpp"

Label *Label::create(const std::string &text,
                     const std::string &font)
{
  Label *ret = new Label();
  if (ret && ret->init(text, font))
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool Label::init(const std::string &text,
                 const std::string &font)
{
  if (!CCNode::init())
    return false;

  m_font = font;
  parseAndBuild(text);
  return true;
}

void Label::setText(const std::string &text)
{
  for (auto lbl : m_parts)
    lbl->removeFromParentAndCleanup(true);
  m_parts.clear();

  parseAndBuild(text);
  updateColor();
}

void Label::setVariant(Variant variant)
{
  m_variant = variant;
  updateColor();
}

Label::Variant Label::getVariant() const
{
  return m_variant;
}

void Label::updateColor()
{
  ccColor3B color;
  switch (m_variant)
  {
  case Variant::Green:
    color = greenColor;
    break;
  case Variant::Orange:
    color = orangeColor;
    break;
  case Variant::Red:
    color = redColor;
    break;
  }

  for (auto lbl : m_parts)
    lbl->setColor(color);
}

void Label::parseAndBuild(const std::string &text)
{
  for (auto lbl : m_parts)
    lbl->removeFromParent();
  m_parts.clear();

  this->setLayout(
      RowLayout::create()
          ->setGap(0.f)
          ->setAxisAlignment(AxisAlignment::Start)
          ->setCrossAxisAlignment(AxisAlignment::Center)
          ->setCrossAxisLineAlignment(AxisAlignment::Start)
          ->setAutoScale(false)
          ->setAutoGrowAxis(true)
          ->setGrowCrossAxis(false));
  this->getLayout()->ignoreInvisibleChildren(true);

  size_t pos = 0;
  bool inSmall = false;

  while (pos < text.size())
  {
    size_t tagOpen = text.find("<small>", pos);
    size_t tagClose = text.find("</small>", pos);

    if (inSmall && tagClose != std::string::npos)
    {
      std::string part = text.substr(pos, tagClose - pos);

      auto label = CCLabelBMFont::create(part.c_str(), m_font.c_str());
      label->setScale(0.3f);
      label->setOpacity(255 * 0.7f);
      label->setAnchorPoint({0, 0.5f});

      this->addChild(label);
      this->updateLayout();
      m_parts.push_back(label);

      pos = tagClose + 8; // skip </small>
      inSmall = false;
    }
    else if (!inSmall && tagOpen != std::string::npos)
    {
      std::string part = text.substr(pos, tagOpen - pos);

      if (!part.empty())
      {
        auto label = CCLabelBMFont::create(part.c_str(), m_font.c_str());
        label->setScale(0.4f);
        label->setAnchorPoint({0, 0.5f});

        this->addChild(label);
        m_parts.push_back(label);
      }

      pos = tagOpen + 7; // skip <small>
      inSmall = true;
    }
    else
    {
      std::string part = text.substr(pos);
      if (!part.empty())
      {
        auto label = CCLabelBMFont::create(part.c_str(), m_font.c_str());
        label->setScale(0.4f);
        label->setAnchorPoint({0, 0.5f});

        this->addChild(label);
        m_parts.push_back(label);
      }
      break;
    }
  }

  this->updateLayout();
}
