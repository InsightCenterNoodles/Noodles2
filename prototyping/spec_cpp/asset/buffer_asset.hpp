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
  // 0: BufferDownloadHint (0 = DEFER, 1 = IMMEDIATE)
  u8 bits = 0;

  constexpr BufferFlags() = default;
  constexpr explicit BufferFlags(u8 raw) : bits(raw) {}

  // Raw
  constexpr u8 raw() const { return bits; }
  constexpr static BufferFlags from_raw(u8 raw) { return BufferFlags(raw); }
  constexpr explicit operator u8() const { return bits; }

  // Download hint
  constexpr BufferDownloadHint download_hint() const {
    return static_cast<BufferDownloadHint>(bits & 0x1u);
  }
  constexpr BufferFlags &set_download_hint(BufferDownloadHint h) {
    bits = (bits & ~0x1u) | (static_cast<u8>(h) & 0x1u);
    return *this;
  }
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
