#pragma once

#include <Geode/Geode.hpp>

#include <string>

// Sets the text and fits the label into maxWidth:
// first shrinks it from scale down to minScale, and if it is still too wide,
// cuts the text from the end and appends "...".
// Cutting removes whole UTF-8 characters and trailing spaces.
void fitLabelWidth(
    cocos2d::CCLabelBMFont *label,
    std::string const &text,
    float maxWidth,
    float scale,
    float minScale);
