#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include "./Label.hpp"

using namespace geode::prelude;

class ToggablePercentagesList : public CCLayer
{
  ScrollLayer *m_scroll = nullptr;
  ScrollLayer *m_endposEditScroll = nullptr;
  std::vector<float> m_startposes;
  std::vector<float> m_enabledStartposes;

public:
  static ToggablePercentagesList *create(CCSize size, std::vector<float> startposes, std::vector<float> disabledStartposes = {});

  void reload();
  void setStartposes(std::vector<float> startposes);
  std::vector<float> getEnabledStartposes();

protected:
  bool init(CCSize size, std::vector<float> startposes, std::vector<float> disabledStartposes);
  void onToggleStartpos(CCObject *);
};