#pragma once

#include "../common.hpp"

// Canonical asset type identifiers
enum class KnownAssetType : u32 {
  BUFFER = 0,
  MESH = 10,
  STD_MATERIAL = 20,
  IMAGE = 6,
};

using BufferAssetID = AssetID;
using MeshAssetID = AssetID;
using ImageAssetID = AssetID;
using StdMaterialAssetID = AssetID;

// ==============================================================================
// Shared: Buffer views

/// @brief A window into a buffer asset
struct BufferView {
  u64 offset;
  u32 length;
  BufferAssetID buffer_asset_id;
};
