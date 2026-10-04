#include "./debugLog.hpp"

#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

bool isDebugMode()
{
  return Mod::get()->getSettingValue<bool>("debug-mode");
}

// Buttons of the "debug-actions" setting in mod.json
$on_mod(Loaded)
{
  ButtonSettingPressedEventV3(Mod::get(), "debug-actions")
      .listen(
          [](std::string_view buttonKey)
          {
            if (buttonKey != "open-logs-folder")
              return;

            if (!file::openFolder(dirs::getGeodeLogDir()))
            {
              Notification::create(
                  "Failed to open the logs folder",
                  NotificationIcon::Error)
                  ->show();
            }
          })
      .leak();
}
