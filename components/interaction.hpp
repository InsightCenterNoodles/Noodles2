#pragma once

#include "../common.hpp"
#include "component.hpp"

// Marker: entity is interactive
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
  u8 flags;
  AxisRange x;
  AxisRange y;
  AxisRange z;
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

// Activation endpoints to call when activated (primary first)
struct ActivateComponent {
  // DYN
  DynamicArray<u8, RpcEndpointID> endpoints;
};
