#pragma once

#include "common.hpp"

using LargeAssetID = u64;

struct LargeAssetRequest {
  LargeAssetID id;
};

struct LargeAssetReply {
  LargeAssetID id;
  u64 asset_size;
  FixedShortString<64> name;
};

struct LargeAssetChunk {
  DynamicByteArray<u64> bytes;
};

struct LargeAssetError {
  DynamicString<u32> error_message;
};