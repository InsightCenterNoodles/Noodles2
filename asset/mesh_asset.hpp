#pragma once

#include "../common.hpp"
#include "asset.hpp"

// ==============================================================================
// Mesh asset (uncompressed representation)

enum class PrimitiveType : u8 {
  Triangles = 0, // only this for now
};

// Bitfield for presence and primitive type
//  0-1: Index size: if 0, missing, otherwise IndexFormat
//  2: Normal?
//  3: Tangent?
//  4: Texture 0?
//  5: Texture 1?
//  6-8: PRIMITIVE_TYPE (PrimitiveType)
using UncompressedFlags = u32;

// Common attribute layout information
struct UncompressedAttribCommon {
  // Byte offset into the view that the attribute starts
  u32 byte_offset;

  // Byte offset between attribute instances
  u16 stride;
};

// Attribute formats (u8)
enum class AttribFormat : u8 {
  VEC2 = 0,      // two float32
  U16VEC2_N = 1, // two u16, normalized
  VEC3 = 2,      // three float32
  VEC4 = 3,      // four float32 (w is tangent handedness)
};

struct PositionAttrib {
  UncompressedAttribCommon common;

  // Must be AttribFormat::VEC3
  AttribFormat format;
};

struct NormalAttrib {
  UncompressedAttribCommon common;

  // Must be AttribFormat::VEC3
  AttribFormat format;
};

struct TangentAttrib {
  UncompressedAttribCommon common;

  // Must be AttribFormat::VEC4
  AttribFormat format;
};

struct TextureAttrib {
  UncompressedAttribCommon common;

  // Must be AttribFormat::VEC2 or AttribFormat::U16VEC2_N
  AttribFormat format;
};

enum class IndexFormat : u8 {
  U8 = 1,
  U16 = 2,
  U32 = 4,
};

struct UncompressedIndex {
  // Byte offset into the view
  u64 byte_offset;

  // Number of index elements
  u32 count;

  // Default: U32 if unspecified
  IndexFormat format;
};

struct UncompressedPatch {
  u32 vertex_count;
  UncompressedFlags flags;

  // Position min then max values (xyz min, then xyz max)
  Vec3 bb_min, bb_max;

  // Always present
  PositionAttrib position;

  // These are always present, BUT they should only be interpreted if the
  // bitflag says they are enabled. Otherwise, they are junk!
  UncompressedIndex index;
  NormalAttrib normal;
  TangentAttrib tangent;
  TextureAttrib texture_0;
  TextureAttrib texture_1;
};

struct UncompressedMesh {
  // Bytes to source mesh content
  BufferView buffer;

  // 'Patches' (aka submeshes) that may use different materials.
  // A mesh with no patches is an error.
  DynamicArray<u8, UncompressedPatch> patches;
};

struct MeshAsset {
  UncompressedMesh uncompressed;
};
