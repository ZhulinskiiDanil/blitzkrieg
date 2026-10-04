#include "./fitLabelWidth.hpp"

void fitLabelWidth(
    cocos2d::CCLabelBMFont *label,
    std::string const &text,
    float maxWidth,
    float scale,
    float minScale)
{
  if (!label)
    return;

  label->setString(text.c_str());
  label->limitLabelWidth(maxWidth, scale, minScale);

  if (label->getScaledContentWidth() <= maxWidth)
    return;

  // Still too wide at the minimum scale: remove characters from the end
  std::string cut = text;

  while (!cut.empty())
  {
    // Remove exactly one UTF-8 character: continuation bytes, then the lead byte
    while (!cut.empty() && (static_cast<unsigned char>(cut.back()) & 0xC0) == 0x80)
      cut.pop_back();

    if (!cut.empty())
      cut.pop_back();

    // "Name ..." looks worse than "Name..."
    while (!cut.empty() && cut.back() == ' ')
      cut.pop_back();

    label->setString((cut + "...").c_str());

    if (label->getScaledContentWidth() <= maxWidth)
      return;
  }
}
