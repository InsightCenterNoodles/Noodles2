#pragma once

#include "../asset/asset.hpp"

// Supported instance payload encodings. The goal is to keep a fat, direct-
// upload format for platforms that prefer it, alongside thin packed variants
// for bandwidth-sensitive cases (e.g., particles). Consumers can pick the
// narrowest format that satisfies their use case; others may emulate by
// expanding to FullFloatInstance on CPU or via a compute pass.
enum class InstanceType : u8 {
  // 64 bytes, direct and simple: float3 position, float4 rotation quaternion,
  // float4 (tu, tv, tsu, tsv) texture transform, float3 non-uniform scale.
  FULL_FLOAT = 0,

  // 32 bytes, quantized but still straightforward to fetch as normalized 16-bit
  // where supported. See ReducedInstance definition below.
  REDUCED = 1,
};

struct InstanceComponent {
  BufferView view;
  u32 count;
  InstanceType type;

  // Optional quantization
  Vec3 spatial_min;
  Vec3 spatial_max;
  float spatial_scale_min;
  float spatial_scale_max;
};

// 64B "fat" format. Preferred when upload size is not a concern or when the
// platform benefits from direct, unpack-free vertex fetch.
struct FullFloatInstance {
  float px, py, pz, _1;
  float qx, qy, qz, qw;
  float tu, tv, tsu, tsv;
  float sx, sy, sz, _2;
};

// 24B format: quantized where it meaningfully reduces bandwidth while retaining
// non-uniform scale and inline UV transforms. Intended for backends that can
// fetch UNORM/SNORM vertex attributes directly and decode in shader with small
// cost. Texture transform is kept, but compressed to 16-bit UNORM [-1, +1] for
// scale and 16-bit SNORM [-1, +1] for translate (common for atlas coords).
// Position is quantized to the per-draw AABB using InstanceComponent
// spatial_min/spatial_max. Rotation is stored as 4×i16 SNORM (normalized unit
// quaternion); consumers should renormalize in-shader to remove drift.
struct ReducedInstance {
  // Position packed as UNORM16 relative to [spatial_min, spatial_max], and
  // scale relative to spatial_scale_min, spatial_scale_max.
  std::array<u16, 4> pxyzs; // px, py, pz, scale

  // Rotation quaternion as SNORM16
  std::array<i16, 4> rot; // qx, qy, qz, qw (SNORM16)

  // UV transform: tsu, tsv in UNORM16; tu, tv in SNORM16
  std::array<u16, 2> ts; // tsu, tsv (UNORM16)
  std::array<i16, 2> tt; // tu, tv (SNORM16)
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(FullFloatInstance);
N2_STATIC_ASSERT_FIXED(ReducedInstance);
