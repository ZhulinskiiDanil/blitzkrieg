#include "playSfx.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/GameManager.hpp>

#include <cstring>

using namespace geode::prelude;

namespace
{
  FMOD::ChannelGroup *sfxGroup = nullptr;

  FMOD_RESULT onNonBlockLoaded(FMOD_SOUND *, FMOD_RESULT)
  {
    return FMOD_OK;
  }

  void applyVolume()
  {
    if (!sfxGroup)
      return;

    const float volume = Mod::get()->getSettingValue<float>("sfx-volume");

    if (Mod::get()->getSettingValue<bool>("ignore-built-in-game-sfx"))
      sfxGroup->setVolume(volume);
    else
      sfxGroup->setVolume(GameManager::get()->m_sfxVolume * volume);
  }

  // A custom file when custom sounds are on and it is set, the bundled one otherwise
  std::string getSoundPath(const char *settingKey, const char *bundled)
  {
    const auto custom = Mod::get()->getSettingValue<std::filesystem::path>(settingKey);

    if (Mod::get()->getSettingValue<bool>("sfx-use-custom-sounds") && !custom.empty())
      return geode::utils::string::pathToString(custom);

    return fmt::format("{}/{}", Mod::get()->getResourcesDir(), bundled);
  }
}

void prepareSfx()
{
  if (!sfxGroup)
    FMODAudioEngine::get()->m_system->createChannelGroup("blitzkrieg", &sfxGroup);

  applyVolume();
}

void playSfx(SfxKind kind)
{
  if (Mod::get()->getSettingValue<bool>("disable-run-notification-sound"))
    return;

  prepareSfx();

  std::string path;

  switch (kind)
  {
  case SfxKind::Stage:
    path = getSoundPath("sfx-stage-path", "stage_complete.mp3");
    break;
  case SfxKind::Achievement:
    // The stage sound unless an achievement sound is chosen
    path = getSoundPath("sfx-achievement-path", "stage_complete.mp3");
    break;
  default:
    path = getSoundPath("sfx-progress-path", "progress_complete.mp3");
    break;
  }

  FMOD_CREATESOUNDEXINFO exinfo;
  std::memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
  exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
  exinfo.nonblockcallback = onNonBlockLoaded;

  auto *system = FMODAudioEngine::get()->m_system;
  FMOD::Sound *sound = nullptr;

  const auto result = system->createStream(
      path.c_str(),
      FMOD_DEFAULT | FMOD_LOOP_OFF | FMOD_2D | FMOD_LOWMEM,
      &exinfo,
      &sound);

  if (result != FMOD_OK)
  {
    log::info("FMOD ERROR {}", static_cast<int>(result));
    return;
  }

  FMOD::Channel *channel = nullptr;

  if (auto played = system->playSound(sound, sfxGroup, false, &channel); played != FMOD_OK)
    log::info("FMOD ERROR STARTING AUDIO: {}", static_cast<int>(played));
}
