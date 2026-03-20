#include "./GraphPointDisplayList.hpp"

GraphPointDisplayList *GraphPointDisplayList::create()
{
  auto ret = new GraphPointDisplayList();
  if (ret && ret->init())
  {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool GraphPointDisplayList::init()
{
  if (!CCLayer::init())
    return false;

  this->ignoreAnchorPointForPosition(false);
  m_padding = {4, 4, 4, 4};

  m_container = CCLayer::create();
  m_container->setPosition({m_padding.left, m_padding.bottom});
  m_container->setAnchorPoint({0, 0});
  m_container->setZOrder(1);
  m_container->setLayout(ColumnLayout::create()
                             ->setGap(4)
                             ->setAutoScale(false)
                             ->setAutoGrowAxis(true));
  this->addChild(m_container);

  m_bg = RectNode::create(this->getContentSize(), ccc4FFromccc4B({0, 0, 0, static_cast<int>(255 * .8f)}), 5 + m_padding.top);
  m_bg->setZOrder(0);
  this->addChild(m_bg);

  return true;
}

void GraphPointDisplayList::updatePointText(const std::string &text, const std::string &pointUUID, bool enabled)
{
  if (!enabled)
  {
    auto it = m_items.find(pointUUID);
    if (it != m_items.end())
    {
      m_container->removeChild(it->second);
      m_items.erase(it);

      m_container->updateLayout();
    }
  }
  else
  {
    if (m_items.contains(pointUUID))
    {
      if (auto node = typeinfo_cast<GraphPointDisplay *>(m_items[pointUUID]))
        node->setText(text);
    }
    else
    {
      auto cell = GraphPointDisplay::create(text);
      m_container->addChild(cell);

      m_items[pointUUID] = cell;
    }

    m_container->updateLayout();
  }

  this->setContentSize({m_container->getContentWidth() + m_padding.left + m_padding.right,
                        m_container->getContentHeight() + m_padding.top + m_padding.bottom});
  m_bg->setSize(this->getContentSize());
}