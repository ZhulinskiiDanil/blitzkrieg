#pragma once

// GD's ScrollLayer clips a bit outside of its bounds on the top, left and
// right sides. Outlines drawn exactly at the edge of a scroll item stick out
// of the list there, so they are inset by this amount on those three sides.
// The bottom side is clipped correctly and needs no inset.
constexpr float SCROLL_CLIP_INSET = 1.5f;
