#pragma once

#include "../common.hpp"
#include "asset.hpp"

// ==============================================================================
// Image asset

enum class ImageSemantic : u8 {
  COLOR = 0, // Color image
  DATA = 1,  // Data/utility image
};

enum class ImageAddressMode : u8 {
  ClampToEdge = 0,
  Repeat = 1,
  MirroredRepeat = 2,
  ClampToBorder = 3,
};

enum class ImageFilterMode : u8 {
  Nearest = 0,
  Linear = 1,
};

enum class ImageSamplerBorderColor : u8 {
  TransparentBlack = 0,
  OpaqueBlack = 1,
  OpaqueWhite = 2,
  Zero = 3,
};

// Sampler flags bitfield (u32)
// bits 0-1: address_mode_u (ImageAddressMode)
// bits 2-3: address_mode_v (ImageAddressMode)
// bits 4-5: address_mode_w (ImageAddressMode)
// bits 6-7: border_color (ImageSamplerBorderColor)
// bit 8: mag_filter (ImageFilterMode)
// bit 9: min_filter (ImageFilterMode)
// bit 10: mipmap_filter (ImageFilterMode)
// other bits reserved
struct SamplerFlags {
  u32 bits = 0;

  constexpr SamplerFlags() = default;
  constexpr explicit SamplerFlags(u32 raw) : bits(raw) {}

  // Raw
  constexpr u32 raw() const { return bits; }
  constexpr static SamplerFlags from_raw(u32 raw) { return SamplerFlags(raw); }
  constexpr explicit operator u32() const { return bits; }

  // Address modes
  constexpr ImageAddressMode address_mode_u() const { return static_cast<ImageAddressMode>((bits >> 0) & 0x3u); }
  constexpr ImageAddressMode address_mode_v() const { return static_cast<ImageAddressMode>((bits >> 2) & 0x3u); }
  constexpr ImageAddressMode address_mode_w() const { return static_cast<ImageAddressMode>((bits >> 4) & 0x3u); }
  constexpr SamplerFlags &set_address_mode_u(ImageAddressMode m) { bits = (bits & ~(0x3u << 0)) | ((static_cast<u32>(m) & 0x3u) << 0); return *this; }
  constexpr SamplerFlags &set_address_mode_v(ImageAddressMode m) { bits = (bits & ~(0x3u << 2)) | ((static_cast<u32>(m) & 0x3u) << 2); return *this; }
  constexpr SamplerFlags &set_address_mode_w(ImageAddressMode m) { bits = (bits & ~(0x3u << 4)) | ((static_cast<u32>(m) & 0x3u) << 4); return *this; }

  // Border color (bits 6-7)
  constexpr ImageSamplerBorderColor border_color() const { return static_cast<ImageSamplerBorderColor>((bits >> 6) & 0x3u); }
  constexpr SamplerFlags &set_border_color(ImageSamplerBorderColor c) { bits = (bits & ~(0x3u << 6)) | ((static_cast<u32>(c) & 0x3u) << 6); return *this; }

  // Filters
  constexpr ImageFilterMode mag_filter() const { return static_cast<ImageFilterMode>((bits >> 8) & 0x1u); }
  constexpr ImageFilterMode min_filter() const { return static_cast<ImageFilterMode>((bits >> 9) & 0x1u); }
  constexpr ImageFilterMode mipmap_filter() const { return static_cast<ImageFilterMode>((bits >> 10) & 0x1u); }
  constexpr SamplerFlags &set_mag_filter(ImageFilterMode m) { bits = (bits & ~(0x1u << 8)) | ((static_cast<u32>(m) & 0x1u) << 8); return *this; }
  constexpr SamplerFlags &set_min_filter(ImageFilterMode m) { bits = (bits & ~(0x1u << 9)) | ((static_cast<u32>(m) & 0x1u) << 9); return *this; }
  constexpr SamplerFlags &set_mipmap_filter(ImageFilterMode m) { bits = (bits & ~(0x1u << 10)) | ((static_cast<u32>(m) & 0x1u) << 10); return *this; }
};

/// @brief Image content and sampling description. (Fixed-only)
/// Data must be PNG, JPG, or KTX2 basisU texture.
struct ImageAsset {
  // Raw bytes of the encoded image
  BufferView data;

  // Sampler behavior flags (see above for bit packing)
  SamplerFlags sampler_flags;

  // Semantic usage of this image
  ImageSemantic semantic;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(ImageAsset);
