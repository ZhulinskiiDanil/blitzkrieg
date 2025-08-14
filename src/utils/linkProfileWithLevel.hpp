#pragma once

#include <string>
#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

/**
 * Привязывает профиль к конкретному уровню и сохраняет его.
 * @param profile Профиль для сохранения.
 * @param level Уровень, к которому привязан профиль.
 */
void linkProfileWithLevel(const Profile &profile, GJGameLevel *level);