#pragma once

#include "../common.hpp"

// Render layer mask used for view filtering, debugging, and role-based views.
// Fixed-only.

struct RenderLayerComponent {
  u32 mask; // bitmask; viewer defines which bits are active per view
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(RenderLayerComponent);

