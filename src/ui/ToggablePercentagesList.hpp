#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include "..\ui\Label.hpp"

using namespace geode::prelude;

class ToggablePercentagesList : public CCLayer {
public:
    static ToggablePercentagesList* create(CCSize size, std::vector<float> startposes);
protected:
    bool init(CCSize size, std::vector<float> startposes);
    void onToggleStartpos(CCObject*);
};