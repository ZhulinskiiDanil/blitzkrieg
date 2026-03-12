#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class EditStartPosPopup : public geode::Popup
{
private:
  CCMenu *m_buttonMenu = nullptr;

  float m_oldStartpos;
  float m_newStartpos;

  TextInput *m_startposInput = nullptr;

  std::function<void(float, float)> m_callback;

  bool init(float startpos, std::function<void(float, float)> callback);

  void onSave(CCObject *);
  void onCancel(CCObject *);

public:
  static EditStartPosPopup *create(float startpos, std::function<void(float, float)> callback);
};