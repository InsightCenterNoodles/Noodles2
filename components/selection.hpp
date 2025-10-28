#pragma once

#include "../common.hpp"

// Selection capabilities and state. Fixed-only.

// Marker: this entity can be selected
struct CanSelectComponent {};

// Marker+style: this entity is currently selected
struct IsSelectedComponent {
  std::array<u8, 4> highlight_color; // sRGB8 RGBA for outlines/highlight
  float width;                        // outline/halo width in world units
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(CanSelectComponent);
N2_STATIC_ASSERT_FIXED(IsSelectedComponent);

