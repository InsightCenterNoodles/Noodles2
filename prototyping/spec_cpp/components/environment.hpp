#pragma once

#include "../asset/asset.hpp"

// Attach an environment/IBL asset to an entity and set per-instance controls.
// Fixed-only.
// Note: intensity default is 1.0 if unspecified; viewers may assume 1.0.
struct EnvironmentComponent {
  EnvironmentAssetID environment; // asset reference
  float intensity;                 // lighting scale (default 1.0)
  UnitQuat rotation;               // per-instance environment rotation
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(EnvironmentComponent);
