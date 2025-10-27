#pragma once

#include "../common.hpp"

// Assign a non-unique text label to an entity (Fixed-only)
struct NameComponent {
  FixedShortString<32> name;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(NameComponent);
