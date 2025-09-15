#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

class StageChangedEvent : public Event
{
protected:
  int m_currentStage;
  int m_totalStages;

public:
  StageChangedEvent(int currentStage, int totalStages)
  {
    m_currentStage = currentStage;
    m_totalStages = totalStages;
  }

  int getCurrentStage() { return m_currentStage; }
  int getTotalStages() { return m_totalStages; }
};