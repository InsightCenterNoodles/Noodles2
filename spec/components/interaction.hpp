#pragma once

#include "../common.hpp"
#include <array>

/// Marker: entity is interactive
/// The size should be zero
struct InteractiveComponent {};

// Axis bounds
struct AxisRange {
  float a;
  float b;
};

// Allowed axes and optional bounds
// flags (u8):
//  bit 0-1: enable bounds for x
//  bit 2-3: enable bounds for y
//  bit 4-5: enable bounds for z
struct AllowedAxis {
  AxisRange x;
  AxisRange y;
  AxisRange z;
  u8 flags;
};

// Translation capability (gated by InteractiveComponent)
struct CanTranslateComponent {
  AllowedAxis axes;
};

// Rotation capability (gated by InteractiveComponent)
struct CanRotateComponent {
  AllowedAxis axes;
};

// Scaling capability (gated by InteractiveComponent)
struct CanScaleComponent {
  AllowedAxis axes;
};

// RPC endpoint identifier used by activation logic
using RpcEndpointID = u32;

// Activation endpoints to call when activated (primary first)
struct ActivateComponent {
  std::array<RpcEndpointID, 8> endpoints;
};
