#pragma once

#include "../common.hpp"
#include "asset.hpp"

// ==============================================================================
// Environment/IBL asset

enum class EnvironmentFormat : u8 { EQUIRECT };

// Fixed parameters for the environment asset.
// Per-instance state (intensity, rotation) lives in EnvironmentComponent to
// allow reuse of the same asset across entities.
//
// Flags (u32):
//  Bit 0: skybox_visible? (0 = hidden, 1 = visible)
//  Bits 1-31: RESERVED
using EnvironmentFlags = u32;

struct EnvironmentAssetFixed {
  EnvironmentFlags flags;
  EnvironmentFormat format; // How the images should be interpreted
};

/// Packed reference
/// bits
/// 0 : active?
/// rest: integer index
using EnvMapIndex = u8;

// Dyn references to images used by the environment
// - image_table holds ImageAssetIDs referenced locally by slot indices
// - env_map: slot for the radiance (environment) map
// - skybox_map: slot for the skybox/background map (may equal env_map)
struct EnvironmentAssetDyn {
  DynamicArray<u8, ImageAssetID> image_table;
  EnvMapIndex env_map;
  EnvMapIndex skybox_map;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(EnvironmentAssetFixed);
