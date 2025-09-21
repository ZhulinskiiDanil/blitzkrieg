#pragma once

#include <Geode/Geode.hpp>
using namespace geode::prelude;

class Label : public CCLayer
{
public:
  enum class Variant
  {
    Green,
    Orange,
    Red
  };

private:
  CCLabelBMFont *m_label = nullptr;
  Variant m_variant = Variant::Orange;

  std::string m_font;
  std::vector<CCLabelBMFont *> m_parts;

  const ccColor3B greenColor = {99, 224, 110};
  const ccColor3B orangeColor = {255, 196, 157};
  const ccColor3B redColor = {253, 106, 106};

  void updateColor();
  void parseAndBuild(const std::string &text);

public:
  static Label *create(const std::string &text,
                       const std::string &font = "gjFont17.fnt");

  bool init(const std::string &text,
            const std::string &font = "gjFont17.fnt");

  CCLabelBMFont *getLabel() const { return m_label; }

  void setText(const std::string &text);

  void setVariant(Variant variant);
  Variant getVariant() const;
};
