#pragma once
#include <Geode/Geode.hpp>
#include <functional>

#include "../../../../store/BackupStore/BackupStore.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// One backup: date, why it was made, its profiles, restore and delete buttons
class BackupCell : public CCNode
{
private:
  static constexpr float HEIGHT = 34.f;
  static constexpr float PADDING = 8.f;

  BackupInfo m_info;

  void onRestore(CCObject *);
  void onDelete(CCObject *);

public:
  std::function<void(BackupInfo const &)> onRestoreRequested;
  std::function<void(BackupInfo const &)> onDeleteRequested;

  static BackupCell *create(BackupInfo const &info, float width);
  bool init(BackupInfo const &info, float width);
};

ccColor3B getBackupReasonColor(BackupReason reason);
