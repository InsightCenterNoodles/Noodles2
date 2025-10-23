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

using BufferSource = MatchGroup<BufferSourceType, LargeAssetID,
                                DynamicString<u16>, DynamicByteArray<u16>>;

/// @brief Where to obtain the raw bytes for a buffer asset and a size hint.
struct BufferAsset {
  // Buffer size hint. Should match final byte count but readers must not rely
  // on it.
  u64 size_hint;

  // DYN

  // Content source
  BufferSource source;
};
