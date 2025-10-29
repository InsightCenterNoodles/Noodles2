#pragma once

#include "../common.hpp"

// Light components are Fixed-only. Orientation/position are taken from the
// entity's Transform/CompressedTransform where applicable.

// Encoded as sRGB8 to keep components compact. Viewers should convert to
// linear for shading and apply the separate HDR intensity scalar.
using SRGB8 = std::array<u8, 3>; // RR GG BB in sRGB

// Light flags (u32)
// Bit 0: casts_shadows?
// Bits 1-31: RESERVED
using LightFlags = u32;

// Directional light: direction from entity rotation; position ignored
struct DirectionalLightComponent {
  SRGB8 color;    // sRGB8 color
  float intensity; // HDR scale factor
  LightFlags flags;
};

// Point light: position from entity transform
struct PointLightComponent {
  SRGB8 color;    // sRGB8 color
  float intensity; // HDR scale factor
  float range;     // influence radius in world units
  LightFlags flags;
};

// Spot light: pose from entity transform
// Angles are in degrees
struct SpotLightComponent {
  SRGB8 color;           // sRGB8 color
  float intensity;        // HDR scale factor
  float range;           // influence radius in world units
  float inner_angle_deg; // full cone inner angle (degrees)
  float outer_angle_deg; // full cone outer angle (degrees)
  LightFlags flags;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(DirectionalLightComponent);
N2_STATIC_ASSERT_FIXED(PointLightComponent);
N2_STATIC_ASSERT_FIXED(SpotLightComponent);
