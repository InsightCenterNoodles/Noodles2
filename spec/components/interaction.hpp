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
struct AllowedAxisFlags {
  // Bit layout per axis: bit N = min, bit N+1 = max
  // X: bits 0-1, Y: bits 2-3, Z: bits 4-5
  u8 bits = 0;

  constexpr AllowedAxisFlags() = default;
  constexpr explicit AllowedAxisFlags(u8 raw) : bits(raw) {}

  // Raw accessors
  constexpr u8 raw() const { return bits; }
  constexpr static AllowedAxisFlags from_raw(u8 raw) { return AllowedAxisFlags(raw); }
  constexpr explicit operator u8() const { return bits; }

  // X
  constexpr bool x_min_enabled() const { return (bits & (1u << 0)) != 0; }
  constexpr bool x_max_enabled() const { return (bits & (1u << 1)) != 0; }
  constexpr AllowedAxisFlags &set_x_min_enabled(bool v) {
    if (v) bits |= (1u << 0); else bits &= ~(1u << 0); return *this;
  }
  constexpr AllowedAxisFlags &set_x_max_enabled(bool v) {
    if (v) bits |= (1u << 1); else bits &= ~(1u << 1); return *this;
  }

  // Y
  constexpr bool y_min_enabled() const { return (bits & (1u << 2)) != 0; }
  constexpr bool y_max_enabled() const { return (bits & (1u << 3)) != 0; }
  constexpr AllowedAxisFlags &set_y_min_enabled(bool v) {
    if (v) bits |= (1u << 2); else bits &= ~(1u << 2); return *this;
  }
  constexpr AllowedAxisFlags &set_y_max_enabled(bool v) {
    if (v) bits |= (1u << 3); else bits &= ~(1u << 3); return *this;
  }

  // Z
  constexpr bool z_min_enabled() const { return (bits & (1u << 4)) != 0; }
  constexpr bool z_max_enabled() const { return (bits & (1u << 5)) != 0; }
  constexpr AllowedAxisFlags &set_z_min_enabled(bool v) {
    if (v) bits |= (1u << 4); else bits &= ~(1u << 4); return *this;
  }
  constexpr AllowedAxisFlags &set_z_max_enabled(bool v) {
    if (v) bits |= (1u << 5); else bits &= ~(1u << 5); return *this;
  }
};

struct AllowedAxis {
  AxisRange x;
  AxisRange y;
  AxisRange z;
  AllowedAxisFlags flags;
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
