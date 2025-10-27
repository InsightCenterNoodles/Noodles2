#pragma once

#include "common.hpp"

using LargeAssetID = u64;

/// Large-asset stream: request by ID (Fixed-only)
/// Protocol semantics (separate stream/channel):
/// - Sender replies with LargeAssetReply (id, total size, name).
/// - Then sends one or more LargeAssetChunk messages whose bytes concatenate
///   to the advertised total size.
/// - Completion is detected by accumulating chunk sizes to match asset_size.
/// - An error terminates the transfer with LargeAssetError.
struct LargeAssetRequestFixed {
  LargeAssetID id;
};
using LargeAssetRequest = LargeAssetRequestFixed;

/// Response header: id, total size, and a short name (Fixed-only)
struct LargeAssetReplyFixed {
  LargeAssetID id;
  u64 asset_size;
  FixedShortString<64> name;
};
using LargeAssetReply = LargeAssetReplyFixed;

/// Stream chunk payload (Dyn-only)
/// - Carries an arbitrary slice of the asset bytes.
struct LargeAssetChunkDyn {
  DynamicByteArray<u64> bytes;
};

/// Error message (Dyn-only)
struct LargeAssetErrorDyn {
  DynamicString<u32> error_message;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(LargeAssetRequestFixed);
N2_STATIC_ASSERT_FIXED(LargeAssetReplyFixed);
