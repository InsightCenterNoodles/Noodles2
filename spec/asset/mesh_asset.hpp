#pragma once

#include "../common.hpp"
#include "asset.hpp"

// ==============================================================================
// Mesh asset (uncompressed representation)
// Split into Fixed (buffer view) and Dyn (patch list)

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
struct UncompressedFlags {
  // Bitfield for presence and primitive type
  //  0-1: Index size: if 0, missing, otherwise IndexFormat code
  //  2: Normal?
  //  3: Tangent?
  //  4: Texture 0?
  //  5: Texture 1?
  //  6-8: PRIMITIVE_TYPE (PrimitiveType)
  u32 bits = 0;

  constexpr UncompressedFlags() = default;
  constexpr explicit UncompressedFlags(u32 raw) : bits(raw) {}

  // Raw accessors
  constexpr u32 raw() const { return bits; }
  constexpr static UncompressedFlags from_raw(u32 raw) { return UncompressedFlags(raw); }
  constexpr explicit operator u32() const { return bits; }

  // Index format code (0 = none)
  constexpr bool has_index() const { return (bits & 0x3u) != 0; }
  constexpr u8 raw_index_code() const { return static_cast<u8>(bits & 0x3u); }
  constexpr UncompressedFlags &set_raw_index_code(u8 code) {
    bits = (bits & ~0x3u) | (static_cast<u32>(code) & 0x3u);
    return *this;
  }

  // Attribute presence
  constexpr bool has_normal() const { return (bits & (1u << 2)) != 0; }
  constexpr bool has_tangent() const { return (bits & (1u << 3)) != 0; }
  constexpr bool has_tex0() const { return (bits & (1u << 4)) != 0; }
  constexpr bool has_tex1() const { return (bits & (1u << 5)) != 0; }
  constexpr UncompressedFlags &set_has_normal(bool v) { if (v) bits |= (1u << 2); else bits &= ~(1u << 2); return *this; }
  constexpr UncompressedFlags &set_has_tangent(bool v) { if (v) bits |= (1u << 3); else bits &= ~(1u << 3); return *this; }
  constexpr UncompressedFlags &set_has_tex0(bool v) { if (v) bits |= (1u << 4); else bits &= ~(1u << 4); return *this; }
  constexpr UncompressedFlags &set_has_tex1(bool v) { if (v) bits |= (1u << 5); else bits &= ~(1u << 5); return *this; }

  // Primitive type (bits 6-8)
  constexpr PrimitiveType primitive_type() const { return static_cast<PrimitiveType>((bits >> 6) & 0x7u); }
  constexpr UncompressedFlags &set_primitive_type(PrimitiveType p) {
    bits = (bits & ~(0x7u << 6)) | ((static_cast<u32>(p) & 0x7u) << 6);
    return *this;
  }
};

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

struct UncompressedMeshFixed {
  // Bytes to source mesh content
  BufferView buffer;
};

struct UncompressedMeshDyn {
  // 'Patches' (aka submeshes) that may use different materials.
  // A mesh with no patches is an error.
  DynamicArray<u8, UncompressedPatch> patches;
};

// Top-level Mesh asset: maps to UncompressedMesh
using MeshAssetFixed = UncompressedMeshFixed;
using MeshAssetDyn = UncompressedMeshDyn;

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(UncompressedMeshFixed);
