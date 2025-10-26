#include "Label.hpp"

Label *Label::create(const std::string &text,
                     const std::string &font,
                     float fontSize)
{
  Label *ret = new Label();
  if (ret && ret->init(text, font, fontSize))
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool Label::init(const std::string &text,
                 const std::string &font,
                 float fontSize)
{
  if (!CCNode::init())
    return false;

  m_fontSize = fontSize;
  m_font = font;
  m_text = text;

  parseAndBuild();
  updateColor();

  return true;
}

void Label::setText(const std::string &text)
{
  for (auto lbl : m_parts)
    lbl->removeFromParentAndCleanup(true);
  m_parts.clear();

  m_text = text;

  parseAndBuild();
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
  default:
    color = defaultColor;
  }

  for (auto lbl : m_parts)
    lbl->setColor(color);
}

void Label::parseAndBuild()
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

  while (pos < m_text.size())
  {
    size_t tagOpen = m_text.find("<small>", pos);
    size_t tagClose = m_text.find("</small>", pos);

    if (inSmall && tagClose != std::string::npos)
    {
      std::string part = m_text.substr(pos, tagClose - pos);

      auto label = CCLabelBMFont::create(part.c_str(), m_font.c_str());
      label->setScale(m_fontSize * 0.75f);
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
      std::string part = m_text.substr(pos, tagOpen - pos);

      if (!part.empty())
      {
        auto label = CCLabelBMFont::create(part.c_str(), m_font.c_str());
        label->setScale(m_fontSize);
        label->setAnchorPoint({0, 0.5f});

        this->addChild(label);
        m_parts.push_back(label);
      }

      pos = tagOpen + 7; // skip <small>
      inSmall = true;
    }
    else
    {
      std::string part = m_text.substr(pos);
      if (!part.empty())
      {
        auto label = CCLabelBMFont::create(part.c_str(), m_font.c_str());
        label->setScale(m_fontSize);
        label->setAnchorPoint({0, 0.5f});

        this->addChild(label);
        m_parts.push_back(label);
      }
      break;
    }
  }

  this->updateLayout();
}

void Label::setFontSize(float fontSize)
{
  m_fontSize = fontSize;
  parseAndBuild();
}