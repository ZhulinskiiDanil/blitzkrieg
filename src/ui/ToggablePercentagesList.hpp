#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include "./Label.hpp"
#include "../popups/EditStartPosPopup/index.hpp"

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
  std::vector<float> getStartposes();
  std::vector<float> getEnabledStartposes();
  bool hasChanged();

protected:
  bool init(CCSize size, std::vector<float> startposes, std::vector<float> disabledStartposes);
  void onToggleStartpos(CCObject *);
  void onEditStartpos(CCObject *);
  void onDeleteStartpos(CCObject *);
};