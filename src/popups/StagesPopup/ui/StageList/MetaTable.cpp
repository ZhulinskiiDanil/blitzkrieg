#include "MetaTable.hpp"

MetaTable *MetaTable::create(const std::vector<MetaData> &data, float width, MetaTablePadding padding)
{
  auto *ret = new MetaTable();
  if (ret && ret->init(data, width, padding))
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool MetaTable::init(const std::vector<MetaData> &data, float width, MetaTablePadding padding)
{
  if (!CCLayer::init())
    return false;

  m_width = width;
  m_padding = padding;

  m_inner = CCNode::create();
  this->addChild(m_inner);

  m_inner->setLayout(
      ColumnLayout::create()
          ->setGap(2.5f)
          ->setAxisAlignment(AxisAlignment::Start)
          ->setCrossAxisAlignment(AxisAlignment::Start)
          ->setCrossAxisLineAlignment(AxisAlignment::Start)
          ->setAutoGrowAxis(true)
          ->setGrowCrossAxis(false));

  m_inner->setPosition({m_padding.left, m_padding.bottom});

  setData(data);
  return true;
}

void MetaTable::setData(const std::vector<MetaData> &data)
{
  for (auto row : m_rows)
    row->removeFromParentAndCleanup(true);
  m_rows.clear();

  for (auto &entry : data)
  {
    auto row = MetaLabel::create(
        m_width - (m_padding.left + m_padding.right),
        entry.label,
        entry.value);
    m_rows.push_back(row);

    m_inner->addChild(row);
    m_inner->updateLayout();
  }

  this->setContentSize({m_width,
                        m_inner->getContentSize().height + m_padding.top + m_padding.bottom});
}
