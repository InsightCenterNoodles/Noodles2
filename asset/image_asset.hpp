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
using SamplerFlags = u32;

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
