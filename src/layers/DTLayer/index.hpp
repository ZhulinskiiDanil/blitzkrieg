#pragma once

#include <Geode/Geode.hpp>
#include "Geode/ui/TextArea.hpp"

using namespace geode::prelude;

class DTLayer : public Popup<GJGameLevel* const&>, public TextInputDelegate, public FLAlertLayerProtocol, public ColorPickPopupDelegate {
  protected:
    bool setup(GJGameLevel* const& Level) override;

  public:
    GJGameLevel* m_Level;
    ScrollLayer* m_ScrollLayer;

    CCNode* alignmentNode;

    static DTLayer* create(GJGameLevel* const& Level);
    
    void onClose(cocos2d::CCObject*) override;
};