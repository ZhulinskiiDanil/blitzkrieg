#pragma once

#include <Geode/Geode.hpp>
#include "MetaLabel.hpp"
#include "../../../../ui/Include.hpp"

using namespace geode::prelude;

struct MetaData
{
  std::string label;
  std::string value;
};

struct MetaTablePadding
{
  float top;
  float right;
  float bottom;
  float left;
};

class MetaTable : public CCLayer
{
private:
  float m_width;
  MetaTablePadding m_padding;
  CCNode *m_inner;
  std::vector<MetaLabel *> m_rows;

public:
  static MetaTable *create(
      const std::vector<MetaData> &data,
      float width = 300.f,
      MetaTablePadding padding = {});
  bool init(
      const std::vector<MetaData> &data,
      float width = 300.f,
      MetaTablePadding padding = {});

  void setData(const std::vector<MetaData> &data);
};
