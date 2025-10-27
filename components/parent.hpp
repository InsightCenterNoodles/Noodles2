#pragma once

#include "../common.hpp"

// The parent entity of this entity (Fixed-only)
struct ChildOfComponent {
  EntityID parent;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(ChildOfComponent);
