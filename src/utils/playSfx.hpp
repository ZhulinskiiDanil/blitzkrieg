#pragma once

enum class SfxKind
{
  // A run was passed
  Progress,
  // A stage was closed
  Stage,
  // An achievement was unlocked or the daily goal was reached
  Achievement
};

// Creates the channel group of the mod and applies the volume settings
void prepareSfx();

// Plays the sound of the kind, custom files when they are set.
// Respects "disable-run-notification-sound".
void playSfx(SfxKind kind);
