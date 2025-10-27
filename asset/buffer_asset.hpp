#pragma once

#include "../common.hpp"
#include "../large_asset.hpp"

// ==============================================================================
// Buffer asset

enum class BufferSourceType : u8 {
  LARGE_ASSET = 0, // Content must be fetched via large-asset channel
  URL = 1,         // UTF-8 URL to fetch content from
  INLINE = 2,      // Inline bytes
};

using BufferSource = DynamicTaggedUnion<
    BufferSourceType,
    Case<BufferSourceType, BufferSourceType::LARGE_ASSET, LargeAssetID>,
    Case<BufferSourceType, BufferSourceType::URL, DynamicString<u16>>,
    Case<BufferSourceType, BufferSourceType::INLINE, DynamicByteArray<u16>>>;

/// Where to obtain the raw bytes for a buffer asset and a size hint.
/// Fixed: size hint only
struct BufferAssetFixed {
  // Buffer size hint. Should match final byte count but readers must not rely
  // on it.
  u64 size_hint;
};

/// Dyn: content source (variant)
struct BufferAssetDyn {
  // Content source
  BufferSource source;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(BufferAssetFixed);
