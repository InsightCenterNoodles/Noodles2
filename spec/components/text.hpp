#pragma once

#include "../common.hpp"

// 3D text label.
// Viewers may choose default font and style.

struct TextComponent {
  FixedShortString<96> text; // UTF-8, null-terminated if shorter
  std::array<u8, 4> color;   // sRGB8 RGBA
  float height;              // nominal text height in world units
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(TextComponent);
