#pragma once

#include "../common.hpp"

// render visibility control (Fixed-only)

enum class Visibility : u8 {
  Inherited,
  Visible,
  Invisible,
};

struct VisibleComponent {
  Visibility visibility = Visibility::Inherited;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(VisibleComponent);
