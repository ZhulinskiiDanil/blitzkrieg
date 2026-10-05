#pragma once
#include <Geode/Geode.hpp>

#include "BackupCell.hpp"
#include "../../../../store/BackupStore/BackupStore.hpp"
#include "../../../../ui/RectNode.hpp"

using namespace geode::prelude;

// Backups tab: every backup of the profiles, newest first,
// with restore, delete, a manual backup and the folder
class BackupsLayer : public CCLayer
{
private:
  static constexpr float SIDE_PADDING = 10.f;
  static constexpr float TOP_PADDING = 12.f;
  static constexpr float BOTTOM_PADDING = 10.f;
  static constexpr float CONTROLS_HEIGHT = 16.f;
  static constexpr float ROW_GAP = 7.f;

  CCSize m_size;

  CCNode *m_summary = nullptr;
  float m_summaryMaxWidth = 0.f;
  ScrollLayer *m_scroll = nullptr;
  CCNode *m_emptyState = nullptr;

  bool m_rebuildQueued = false;

  void drawControls(float y);
  void drawList(float top);
  void updateSummary(std::vector<BackupInfo> const &backups);

  void rebuildList();
  // On the next frame, never inside a cell callback
  void queueRebuild();

  void restore(BackupInfo const &info);
  void remove(BackupInfo const &info);

  void onBackUpNow(CCObject *);
  void onOpenFolder(CCObject *);

public:
  static BackupsLayer *create(const CCSize &size);
  bool init(const CCSize &size);
};
