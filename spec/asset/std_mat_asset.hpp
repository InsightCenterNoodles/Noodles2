#pragma once

#include "../common.hpp"
#include "asset.hpp"

// ==============================================================================
// Standard (PBR) material asset

/// Option flags for this material (u32)
/// Bit 0: double_sided?
/// Bit 1: unlit?
/// Bit 2: fog DISabled?
/// Bits 3-4: FACE
/// Bits 5-6: ALPHA_MODE
/// Bit 7: Color texture?
/// Bit 8: ORM texture?
/// Bit 9: Normal map?
/// Bit 10 : Extended?
/// Bits 11-31: RESERVED
using MatFlags = u32;

// Sub-types
using SRGBA8 = std::array<u8, 4>;        // RR GG BB AA
using LinearRGB = std::array<float, 3>;  // linear RGB, range [0.0, inf)
using LinearRGBA = std::array<float, 4>; // linear RGBA, range [0.0, inf)
using NormalizedUShort = u16;            // normalized integer in [0, 65535]

using ImageTableID = u8; // Note that 0 is an invalid entry!

/// Face culling
enum class Face : u8 { Front = 0, Back = 1, None = 2 };

/// Alpha modes
enum class AlphaMode : u8 {
  Opaque = 0,
  Mask = 1,
  Blend = 2,
  Premultiplied = 3
};

/// UV channel selector (default 0)
using UVChannel = u8; // 0 or 1

// Packed texture reference
// bits
// 0 : active?
// 1-2 : uv channel (adding space if we want to go to 4)
// 3-6 : image table id (4 bits for 16 slots)
using TextureReference = u8;

// Common Occlusion-Roughness-Metallic map, stored as normalized RGB
using ORMMap = TextureReference;

struct Emission {
  /// Default: [0,0,0,1]
  LinearRGB color;

  /// Emissive exposure weight. Range [0.0, 1.0], Default: 1.0
  NormalizedUShort exposure;

  TextureReference texture;
};

struct Transmission {
  // Diffuse transmission. Range [0.0, 1.0], Default: 0.0
  NormalizedUShort diffuse;

  // Specular transmission. Range [0.0, 1.0], Default: 0.0
  NormalizedUShort specular;

  TextureReference diffuse_map;
  TextureReference specular_map;
};

struct Thickness {
  // Thickness. Range: [0.0, inf), Default: 1.0
  float thickness;

  // Attenuation distance. Default: inf (no attenuation)
  float attenuation_distance;

  // Attenuation color. Default: [1,1,1,1]
  SRGBA8 attenuation_color;

  // Optional thickness texture
  TextureReference texture;
};

struct Clearcoat {
  // Clearcoat [0,1], Default: 0
  NormalizedUShort amount;

  // Clearcoat roughness [0,1], Default: 0
  NormalizedUShort roughness;

  /// All clearcoat assets share the same UV channel if specified
  /// bits
  /// 0     : amount_map?
  /// 1     : roughness_map?
  /// 2     : normal_map?
  /// 3-4   : uv channel
  /// 6-10  : amount_map image table id
  /// 11-14 : roughness_map image table id
  /// 15-18 : normal_map image table id
  u32 clearcoat_image_pack;
};

struct Parallax {
  // Depth bias. Default: 0.0
  float depth_bias;

  // Parallax depth scale. Default: 0.05
  float depth_scale;

  // Max parallax layer count. Default: 32.0
  u8 max_layers;

  TextureReference depth_map; // required if Parallax present
};

struct Anisotropy {
  // Strength [0,1], Default: 0
  NormalizedUShort strength;

  // Rotation [0,1], Default: 0
  NormalizedUShort rotation;

  TextureReference texture; // optional
};

// 2D affine transform; column-major 2x2 then translation (x,y)
struct Affine2 {
  std::array<float, 4> mat2x2; // column-major
  float translate_x;
  float translate_y;
};

struct ExtendedMaterial {
  /// Bit 0: Thickness?
  /// Bit 1: Transmission?
  /// Bit 2: Clearcoat?
  /// Bit 3: Anisotropy?
  /// Bit 4: Parallax?
  /// Bit 5: Emission?
  /// Bit 6: UV1 transform?
  /// Bit 7: UV2 transform?
  u8 flags;

  // Optional thickness
  Thickness thickness;

  // Optional transmission
  Transmission transmission;

  // Optional clearcoat
  Clearcoat clearcoat;

  // Optional anisotropy
  Anisotropy anisotropy;

  // Optional parallax
  Parallax parallax;

  // Optional emissive control
  Emission emission;

  // Optional UV transforms (default identity)
  Affine2 uv1_transform;
  Affine2 uv2_transform;
};

/// Standard PBR material
/// Fixed: scalar fields and packed texture refs
struct StdMaterialAssetFixed {
  // Bit flags
  MatFlags flags;

  // Base color (sRGB)
  SRGBA8 base_color;

  // Perceptual roughness [0,1] (normalized u16)
  NormalizedUShort roughness;

  // Metallic [0,1] (normalized u16)
  NormalizedUShort metallic;

  // Index of Refraction (IOR). Normalized between 1-5, x = ((IOR - 1.0) / 5.0)
  // * 65535
  u16 ior;

  // Optional color texture
  TextureReference color_map;

  // Optional ORM texture
  ORMMap orm_map;

  // Optional normal mapping
  TextureReference normal_map;
};

/// Dyn: image table and optional extensions
/// - Presence of `extended` is gated by MatFlags bit 10 in
/// StdMaterialAssetFixed.
struct StdMaterialAssetDyn {
  // Image table
  DynamicArray<u8, ImageAssetID> image_table;

  N2_NO_UNIQUE DynamicOptional<ExtendedMaterial> extended;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(StdMaterialAssetFixed);
