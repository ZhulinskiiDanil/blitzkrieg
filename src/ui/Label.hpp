#pragma once

#include <Geode/Geode.hpp>
using namespace geode::prelude;

class Label : public CCLayer
{
public:
  enum class Variant
  {
    Default,
    Green,
    Orange,
    Red
  };

private:
  CCLabelBMFont *m_label = nullptr;
  Variant m_variant = Variant::Default;

  std::string m_font;
  std::vector<CCLabelBMFont *> m_parts;
  std::string m_text;
  float m_fontSize = .4f;

  const ccColor3B defaultColor = {255, 255, 255};
  const ccColor3B greenColor = {99, 224, 110};
  const ccColor3B orangeColor = {255, 196, 157};
  const ccColor3B redColor = {253, 106, 106};

  void updateColor();
  void parseAndBuild();

public:
  static Label *create(const std::string &text,
                       const std::string &font = "gjFont17.fnt",
                       float fontSize = 1.f);
  bool init(const std::string &text,
            const std::string &font = "gjFont17.fnt",
            float fontSize = 1.f);

  CCLabelBMFont *getLabel() const { return m_label; }
  Variant getVariant() const;

  void setText(const std::string &text);
  void setFontSize(float fontSize);
  void setVariant(Variant variant);
};
