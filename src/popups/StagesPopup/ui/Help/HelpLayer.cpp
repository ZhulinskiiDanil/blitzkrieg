#include "HelpLayer.hpp"

HelpLayer *HelpLayer::create(const CCSize &contentSize)
{
  auto ret = new HelpLayer();
  if (ret && ret->init(contentSize))
  {
    ret->autorelease();
    return ret;
  }

  CC_SAFE_DELETE(ret);
  return nullptr;
}

bool HelpLayer::init(const CCSize &contentSize)
{
  if (!CCLayer::init())
    return false;

  Padding padding{15.f, 15.f, 15.f, 15.f}; // top, bottom, left, right

  m_size = contentSize;
  this->setContentSize(m_size);

  const auto innerSize = CCSize(
      m_size.width - padding.left - padding.right,
      m_size.height - padding.top - padding.bottom);

  // ! --- ScrollLayer --- !
  m_scroll = ScrollLayer::create(innerSize);
  m_scroll->setPosition({padding.left, padding.bottom});

  m_scroll->m_contentLayer->setLayout(
      ColumnLayout::create()
          ->setGap(2.5f)
          ->setAxisReverse(true)
          ->setAxisAlignment(AxisAlignment::End)
          ->setAutoGrowAxis(m_scroll->getContentHeight()));

  this->addChild(m_scroll);

  // ! --- BG --- !
  RectNode *bg = RectNode::create(innerSize, ccc4FFromccc4B({30, 30, 30, 255}), 8);
  bg->ignoreAnchorPointForPosition(false);
  bg->setAnchorPoint({0.5f, 0.5f});
  bg->setPosition({innerSize.width / 2 + padding.left, innerSize.height / 2 + padding.bottom});
  bg->setZOrder(-1);
  this->addChild(bg);

  // ! --- Borders --- !
  auto borders = ListBorders::create();
  borders->setSpriteFrames("list-top.png"_spr, "list-side.png"_spr, 2.f);
  borders->updateLayout();
  borders->setContentSize({innerSize.width, innerSize.height - 3});
  borders->setPosition({innerSize.width / 2 + padding.left, innerSize.height / 2 - .5f + padding.bottom});
  borders->setAnchorPoint({0.5f, 0.5f});
  this->addChild(borders);

  // ! Set borders color to dark gray
  for (auto child : CCArrayExt<CCNodeRGBA *>(borders->getChildren()))
    child->setColor(ccc3(50, 50, 50));

  return true;
}