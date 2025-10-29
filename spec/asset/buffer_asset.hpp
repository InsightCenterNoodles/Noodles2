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

enum class BufferDownloadHint : u8 {
  DEFER = 0, // Readers may obtain the buffer (if URL or LARGE ASSET) on demand
  IMMEDIATE = 1, // Readers should obtain the buffer immediately
};

struct BufferFlags {
  // Bits
  // 0: BufferDownloadHint
  u8 content;
};

/// Where to obtain the raw bytes for a buffer asset and a size hint.
struct BufferAssetFixed {
  // Buffer size. This MUST be the exact size of the buffer.
  u64 size;

  // Flags
  BufferFlags flags;
};

/// Dyn: content source (variant)
struct BufferAssetDyn {
  // Content source
  BufferSource source;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(BufferAssetFixed);
