#include "getKeybindText.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

std::string getKeybindText(const char *settingKey)
{
  auto keybinds = Mod::get()->getSettingValue<std::vector<Keybind>>(settingKey);
  return keybinds.empty() ? std::string() : keybinds.front().toString();
}
