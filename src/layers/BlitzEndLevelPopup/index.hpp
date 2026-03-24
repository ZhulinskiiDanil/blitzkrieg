#include <Geode/modify/EndLevelLayer.hpp>

#include "../../popups/StagesPopup/index.hpp"
#include "../../utils/selectJsonFile.hpp"

using namespace geode::prelude;

class $modify(BlitzEndLevelLayer, EndLevelLayer)
{
private:
    void onPopup();

public:
    void customSetup();
};
