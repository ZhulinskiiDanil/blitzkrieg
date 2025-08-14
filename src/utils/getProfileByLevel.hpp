#pragma once

#include <string>
#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include "../serialization/profile/index.hpp"
#include "./getProfiles.hpp"

using namespace geode::prelude;

/**
 * Получает профиль, привязанный к уровню.
 * @param level Уровень, к которому привязан профиль.
 * @return Profile объект, если найден, иначе пустой Profile.
 */
Profile getProfileByLevel(GJGameLevel *level);

/**
 * Получает профиль по строковому идентификатору уровня.
 * @param levelId Уникальный ID уровня.
 * @return Profile объект, если найден, иначе пустой Profile.
 */
Profile getProfileByLevel(const std::string &levelId);