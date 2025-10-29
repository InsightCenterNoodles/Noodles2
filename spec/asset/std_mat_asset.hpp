#pragma once

#include "../common.hpp"
#include "asset.hpp"

// ==============================================================================
// Standard (PBR) material asset

/// Option flags for this material (u32)
/// See MatFlags struct below for detailed field accessors.

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
struct MatFlags {
  u32 bits = 0;

  constexpr MatFlags() = default;
  constexpr explicit MatFlags(u32 raw) : bits(raw) {}

  // Raw accessors
  constexpr u32 raw() const { return bits; }
  constexpr static MatFlags from_raw(u32 raw) { return MatFlags(raw); }
  constexpr explicit operator u32() const { return bits; }

  // Single-bit flags
  constexpr bool double_sided() const { return (bits & (1u << 0)) != 0; }
  constexpr bool unlit() const { return (bits & (1u << 1)) != 0; }
  constexpr bool fog_disabled() const { return (bits & (1u << 2)) != 0; }
  constexpr bool has_color_texture() const { return (bits & (1u << 7)) != 0; }
  constexpr bool has_orm_texture() const { return (bits & (1u << 8)) != 0; }
  constexpr bool has_normal_map() const { return (bits & (1u << 9)) != 0; }
  constexpr bool has_extended() const { return (bits & (1u << 10)) != 0; }

  constexpr MatFlags &set_double_sided(bool v) {
    if (v)
      bits |= (1u << 0);
    else
      bits &= ~(1u << 0);
    return *this;
  }
  constexpr MatFlags &set_unlit(bool v) {
    if (v)
      bits |= (1u << 1);
    else
      bits &= ~(1u << 1);
    return *this;
  }
  constexpr MatFlags &set_fog_disabled(bool v) {
    if (v)
      bits |= (1u << 2);
    else
      bits &= ~(1u << 2);
    return *this;
  }
  constexpr MatFlags &set_has_color_texture(bool v) {
    if (v)
      bits |= (1u << 7);
    else
      bits &= ~(1u << 7);
    return *this;
  }
  constexpr MatFlags &set_has_orm_texture(bool v) {
    if (v)
      bits |= (1u << 8);
    else
      bits &= ~(1u << 8);
    return *this;
  }
  constexpr MatFlags &set_has_normal_map(bool v) {
    if (v)
      bits |= (1u << 9);
    else
      bits &= ~(1u << 9);
    return *this;
  }
  constexpr MatFlags &set_has_extended(bool v) {
    if (v)
      bits |= (1u << 10);
    else
      bits &= ~(1u << 10);
    return *this;
  }

  // Multi-bit fields
  // Face (bits 3-4)
  constexpr Face face() const { return static_cast<Face>((bits >> 3) & 0x3u); }
  constexpr MatFlags &set_face(Face f) {
    bits = (bits & ~(0x3u << 3)) | ((static_cast<u32>(f) & 0x3u) << 3);
    return *this;
  }

  // Alpha mode (bits 5-6)
  constexpr AlphaMode alpha_mode() const {
    return static_cast<AlphaMode>((bits >> 5) & 0x3u);
  }
  constexpr MatFlags &set_alpha_mode(AlphaMode a) {
    bits = (bits & ~(0x3u << 5)) | ((static_cast<u32>(a) & 0x3u) << 5);
    return *this;
  }
};

/// UV channel selector (default 0)
using UVChannel = u8; // 0 or 1

// Packed texture reference
// bits
// 0 : active?
// 1-2 : uv channel (adding space if we want to go to 4)
// 3-6 : image table id (4 bits for 16 slots)
struct TextureReference {
  u8 bits = 0;

  constexpr TextureReference() = default;
  constexpr explicit TextureReference(u8 raw) : bits(raw) {}

  // Raw
  constexpr u8 raw() const { return bits; }
  constexpr static TextureReference from_raw(u8 raw) {
    return TextureReference(raw);
  }
  constexpr explicit operator u8() const { return bits; }

  // Active flag
  constexpr bool active() const { return (bits & 0x1u) != 0; }
  constexpr TextureReference &set_active(bool v) {
    if (v)
      bits |= 0x1u;
    else
      bits &= ~0x1u;
    return *this;
  }

  // UV channel (bits 1-2)
  constexpr UVChannel uv_channel() const {
    return static_cast<UVChannel>((bits >> 1) & 0x3u);
  }
  constexpr TextureReference &set_uv_channel(UVChannel ch) {
    bits = (bits & ~(0x3u << 1)) | ((static_cast<u8>(ch) & 0x3u) << 1);
    return *this;
  }

  // Image table id (bits 3-6)
  constexpr ImageTableID image_table_id() const {
    return static_cast<ImageTableID>((bits >> 3) & 0xFu);
  }
  constexpr TextureReference &set_image_table_id(ImageTableID id) {
    bits = (bits & ~(0xFu << 3)) | ((static_cast<u8>(id) & 0xFu) << 3);
    return *this;
  }
};

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

// Clearcoat shared image packing (u32)
// bits
// 0     : amount_map?
// 1     : roughness_map?
// 2     : normal_map?
// 3-4   : uv channel
// 6-10  : amount_map image table id
// 11-14 : roughness_map image table id
// 15-18 : normal_map image table id
struct ClearcoatImagePack {
  u32 bits = 0;

  constexpr ClearcoatImagePack() = default;
  constexpr explicit ClearcoatImagePack(u32 raw) : bits(raw) {}

  // Raw helpers
  constexpr u32 raw() const { return bits; }
  constexpr static ClearcoatImagePack from_raw(u32 raw) {
    return ClearcoatImagePack(raw);
  }
  constexpr explicit operator u32() const { return bits; }

  // Presence bits
  constexpr bool has_amount_map() const { return (bits & (1u << 0)) != 0; }
  constexpr bool has_roughness_map() const { return (bits & (1u << 1)) != 0; }
  constexpr bool has_normal_map() const { return (bits & (1u << 2)) != 0; }
  constexpr ClearcoatImagePack &set_has_amount_map(bool v) {
    if (v)
      bits |= (1u << 0);
    else
      bits &= ~(1u << 0);
    return *this;
  }
  constexpr ClearcoatImagePack &set_has_roughness_map(bool v) {
    if (v)
      bits |= (1u << 1);
    else
      bits &= ~(1u << 1);
    return *this;
  }
  constexpr ClearcoatImagePack &set_has_normal_map(bool v) {
    if (v)
      bits |= (1u << 2);
    else
      bits &= ~(1u << 2);
    return *this;
  }

  // Shared UV channel (bits 3-4)
  constexpr UVChannel uv_channel() const {
    return static_cast<UVChannel>((bits >> 3) & 0x3u);
  }
  constexpr ClearcoatImagePack &set_uv_channel(UVChannel ch) {
    bits = (bits & ~(0x3u << 3)) | ((static_cast<u32>(ch) & 0x3u) << 3);
    return *this;
  }

  // Image table IDs
  constexpr ImageTableID amount_map_image_table_id() const {
    return static_cast<ImageTableID>((bits >> 6) & 0x1Fu);
  }
  constexpr ImageTableID roughness_map_image_table_id() const {
    return static_cast<ImageTableID>((bits >> 11) & 0x0Fu);
  }
  constexpr ImageTableID normal_map_image_table_id() const {
    return static_cast<ImageTableID>((bits >> 15) & 0x0Fu);
  }
  constexpr ClearcoatImagePack &set_amount_map_image_table_id(ImageTableID id) {
    bits = (bits & ~(0x1Fu << 6)) | ((static_cast<u32>(id) & 0x1Fu) << 6);
    return *this;
  }
  constexpr ClearcoatImagePack &
  set_roughness_map_image_table_id(ImageTableID id) {
    bits = (bits & ~(0x0Fu << 11)) | ((static_cast<u32>(id) & 0x0Fu) << 11);
    return *this;
  }
  constexpr ClearcoatImagePack &set_normal_map_image_table_id(ImageTableID id) {
    bits = (bits & ~(0x0Fu << 15)) | ((static_cast<u32>(id) & 0x0Fu) << 15);
    return *this;
  }
};

struct Clearcoat {
  // Clearcoat [0,1], Default: 0
  NormalizedUShort amount;

  // Clearcoat roughness [0,1], Default: 0
  NormalizedUShort roughness;

  /// All clearcoat assets share the same UV channel if specified
  ClearcoatImagePack clearcoat_image_pack;
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

struct ExtendedMaterialFlags {
  // Bit layout:
  // 0: Thickness, 1: Transmission, 2: Clearcoat, 3: Anisotropy,
  // 4: Parallax, 5: Emission, 6: UV1 transform, 7: UV2 transform
  u8 bits = 0;

  constexpr ExtendedMaterialFlags() = default;
  constexpr explicit ExtendedMaterialFlags(u8 raw) : bits(raw) {}

  // Raw
  constexpr u8 raw() const { return bits; }
  constexpr static ExtendedMaterialFlags from_raw(u8 raw) {
    return ExtendedMaterialFlags(raw);
  }
  constexpr explicit operator u8() const { return bits; }

  // Getters
  constexpr bool has_thickness() const { return (bits & (1u << 0)) != 0; }
  constexpr bool has_transmission() const { return (bits & (1u << 1)) != 0; }
  constexpr bool has_clearcoat() const { return (bits & (1u << 2)) != 0; }
  constexpr bool has_anisotropy() const { return (bits & (1u << 3)) != 0; }
  constexpr bool has_parallax() const { return (bits & (1u << 4)) != 0; }
  constexpr bool has_emission() const { return (bits & (1u << 5)) != 0; }
  constexpr bool has_uv1_transform() const { return (bits & (1u << 6)) != 0; }
  constexpr bool has_uv2_transform() const { return (bits & (1u << 7)) != 0; }

  // Setters
  constexpr ExtendedMaterialFlags &set_has_thickness(bool v) {
    if (v)
      bits |= (1u << 0);
    else
      bits &= ~(1u << 0);
    return *this;
  }
  constexpr ExtendedMaterialFlags &set_has_transmission(bool v) {
    if (v)
      bits |= (1u << 1);
    else
      bits &= ~(1u << 1);
    return *this;
  }
  constexpr ExtendedMaterialFlags &set_has_clearcoat(bool v) {
    if (v)
      bits |= (1u << 2);
    else
      bits &= ~(1u << 2);
    return *this;
  }
  constexpr ExtendedMaterialFlags &set_has_anisotropy(bool v) {
    if (v)
      bits |= (1u << 3);
    else
      bits &= ~(1u << 3);
    return *this;
  }
  constexpr ExtendedMaterialFlags &set_has_parallax(bool v) {
    if (v)
      bits |= (1u << 4);
    else
      bits &= ~(1u << 4);
    return *this;
  }
  constexpr ExtendedMaterialFlags &set_has_emission(bool v) {
    if (v)
      bits |= (1u << 5);
    else
      bits &= ~(1u << 5);
    return *this;
  }
  constexpr ExtendedMaterialFlags &set_has_uv1_transform(bool v) {
    if (v)
      bits |= (1u << 6);
    else
      bits &= ~(1u << 6);
    return *this;
  }
  constexpr ExtendedMaterialFlags &set_has_uv2_transform(bool v) {
    if (v)
      bits |= (1u << 7);
    else
      bits &= ~(1u << 7);
    return *this;
  }
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
  ExtendedMaterialFlags flags;

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
