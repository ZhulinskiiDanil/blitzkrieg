#pragma once

#include <string>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

// class StagesPopupDelegate {
// public:
//   virtual void onFolderCreate(std::string);
// };

class StagesPopup : public geode::Popup<>
{
private:
  // StagesPopupDelegate* m_delegate = nullptr;
  TextInput *m_input = nullptr;

  bool setup() override;

  void onCreate(CCObject *);

public:
  // static StagesPopup* create(StagesPopupDelegate*);
  static StagesPopup *create();
};