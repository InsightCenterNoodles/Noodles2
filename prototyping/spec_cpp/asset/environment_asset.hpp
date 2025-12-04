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
struct EnvironmentFlags {
  u32 bits = 0;

  constexpr EnvironmentFlags() = default;
  constexpr explicit EnvironmentFlags(u32 raw) : bits(raw) {}

  // Raw
  constexpr u32 raw() const { return bits; }
  constexpr static EnvironmentFlags from_raw(u32 raw) { return EnvironmentFlags(raw); }
  constexpr explicit operator u32() const { return bits; }

  // Skybox visibility
  constexpr bool skybox_visible() const { return (bits & 0x1u) != 0; }
  constexpr EnvironmentFlags &set_skybox_visible(bool v) { if (v) bits |= 0x1u; else bits &= ~0x1u; return *this; }
};

struct EnvironmentAssetFixed {
  EnvironmentFlags flags;
  EnvironmentFormat format; // How the images should be interpreted
};

/// Packed reference
/// bits
/// 0 : active?
/// 1-7 : integer index
struct EnvMapIndex {
  u8 bits = 0;

  constexpr EnvMapIndex() = default;
  constexpr explicit EnvMapIndex(u8 raw) : bits(raw) {}

  // Raw
  constexpr u8 raw() const { return bits; }
  constexpr static EnvMapIndex from_raw(u8 raw) { return EnvMapIndex(raw); }
  constexpr explicit operator u8() const { return bits; }

  // Fields
  constexpr bool active() const { return (bits & 0x1u) != 0; }
  constexpr EnvMapIndex &set_active(bool v) { if (v) bits |= 0x1u; else bits &= ~0x1u; return *this; }

  constexpr u8 index() const { return static_cast<u8>((bits >> 1) & 0x7Fu); }
  constexpr EnvMapIndex &set_index(u8 idx) {
    bits = (bits & ~(0x7Fu << 1)) | ((static_cast<u8>(idx) & 0x7Fu) << 1);
    return *this;
  }
};

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
