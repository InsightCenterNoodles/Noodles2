#pragma once

#include "../common.hpp"

// Billboard behavior for orienting an entity towards the viewer.
// Fixed-only. The viewer/platform chooses how to interpret the mode.

enum class BillboardMode : u8 {
  Spherical = 0,   // faces camera in all axes
  Cylindrical = 1, // faces camera around vertical axis; keeps up-vector
};

struct BillboardComponent {
  BillboardMode mode;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(BillboardComponent);

