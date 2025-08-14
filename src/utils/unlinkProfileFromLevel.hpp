#pragma once

#include <string>
#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include "../serialization/profile/index.hpp"

using namespace geode::prelude;

/**
 * Удаляет привязку профиля к уровню.
 * @param profile Профиль для удаления.
 * @param level Уровень, от которого отвязывается профиль.
 */
void unlinkProfileFromLevel(const Profile &profile, GJGameLevel *level);

/**
 * Удаляет привязку профиля к уровню по его идентификатору.
 * @param profile Профиль для удаления.
 * @param levelId Идентификатор уровня.
 */
void unlinkProfileFromLevel(const Profile &profile, const std::string &levelId);
