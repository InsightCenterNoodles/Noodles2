#pragma once

#include "../common.hpp"
#include <cmath>

// The position of this entity, relative to it's parent, if one exists
// (see CHILD_OF_COMPONENT). If there is no parent, then it is a world-relative
// transform.
//
// There are two kinds of transforms, compressed and non compressed. While valid
// to have both on an entity, it is considered bad form, and only the last of
// the two kinds of transform applied is valid.

// Position/rotation/scale relative to parent (or world if no parent)
// (Fixed-only)
struct TransformComponent {
  Vec3 position;
  UnitQuat rotation;
  Vec3 scale;
};

// Compressed transform form

// A compressed unit quaternion using the "smallest three" method.
//
// This format stores 3 components of a unit quaternion, using 9 bits each,
// and drops the component with the largest absolute value. That missing
// component is reconstructed during decoding using the identity x² + y² + z² +
// w² = 1.
//
// The quaternion is canonicalized such that the largest component is always
// positive. This ensures the reconstructed component is unambiguous.
//
// Bits are packed as follows (from MSB to LSB):
//
// [31..30] = INDEX (2 bits)
//     Index of the largest component (0 = x, 1 = y, 2 = z, 3 = w)
//
// [29..21] = COMPONENT_0 (9 bits)
// [20..12] = COMPONENT_1 (9 bits)
// [11..03] = COMPONENT_2 (9 bits)
//
// The remaining bit (bit 0) is unused and should be zero.
//
// Each component is quantized from the range [-1/√2, +1/√2] (~[-0.7071,
// +0.7071]) into 9 bits (512 levels). Dequantization restores the component
// values, and the missing component is reconstructed with:
//
//     missing = sqrt(1.0 - (c0² + c1² + c2²))
//
// This achieves ~29-bit storage per quaternion, down from 128 bits.

using CompressedQuat = u32; // Smallest-three encoding
using CompressedVec3 = std::array<u16, 3>;

// Compressed transform (Fixed-only)
struct CompressedTransformComponent {
  CompressedVec3 position;
  CompressedQuat rotation;
  float uniform_scale;
};

// Quantize a unit quaternion using the smallest-three scheme described above.
// Returns a 32-bit packed value with layout:
// [31..30]=index, [29..21]=c0, [20..12]=c1, [11..03]=c2, [2..0]=0
inline CompressedQuat compress_quat(const UnitQuat &q_in) {
  // Normalize defensively (UnitQuat is expected normalized, but be robust)
  float x = q_in[0], y = q_in[1], z = q_in[2], w = q_in[3];
  const float n2 = x * x + y * y + z * z + w * w;
  if (n2 > 0.0f && std::abs(n2 - 1.0f) > 1e-6f) {
    const float invn = 1.0f / std::sqrt(n2);
    x *= invn;
    y *= invn;
    z *= invn;
    w *= invn;
  }

  // Find index of largest magnitude component
  float ax = std::abs(x), ay = std::abs(y), az = std::abs(z), aw = std::abs(w);
  u32 idx = 0;
  float amax = ax;
  if (ay > amax) {
    idx = 1;
    amax = ay;
  }
  if (az > amax) {
    idx = 2;
    amax = az;
  }
  if (aw > amax) {
    idx = 3;
    amax = aw;
  }

  // Canonicalize so the largest component is non-negative
  float sign = 1.0f;
  switch (idx) {
  case 0:
    if (x < 0)
      sign = -1.0f;
    break;
  case 1:
    if (y < 0)
      sign = -1.0f;
    break;
  case 2:
    if (z < 0)
      sign = -1.0f;
    break;
  case 3:
    if (w < 0)
      sign = -1.0f;
    break;
  }
  if (sign < 0.0f) {
    x = -x;
    y = -y;
    z = -z;
    w = -w;
  }

  // Collect the three non-max components in fixed order (x,y,z,w skipping idx)
  float comps[3];
  {
    u32 k = 0;
    if (idx != 0)
      comps[k++] = x;
    if (idx != 1)
      comps[k++] = y;
    if (idx != 2)
      comps[k++] = z;
    if (idx != 3)
      comps[k++] = w;
  }

  // Quantize from [-1/sqrt(2), +1/sqrt(2)] to 9 bits
  const float inv_sqrt2 = 0.70710678118654752440f; // 1/sqrt(2)
  const float lo = -inv_sqrt2;
  const float hi = inv_sqrt2;
  const float range = hi - lo; // == sqrt(2)

  auto quant9 = [&](float v) -> u32 {
    // Clamp to representable range
    if (v < lo)
      v = lo;
    if (v > hi)
      v = hi;
    float t = (v - lo) / range;                        // [0,1]
    u32 q = static_cast<u32>(std::lround(t * 511.0f)); // 9 bits
    if (q > 511u)
      q = 511u;
    return q;
  };

  u32 q0 = quant9(comps[0]);
  u32 q1 = quant9(comps[1]);
  u32 q2 = quant9(comps[2]);

  // Pack bits: index at [31..30], components at [29..21],[20..12],[11..03]
  CompressedQuat packed = 0;
  packed |= (idx & 0x3u) << 30;
  packed |= (q0 & 0x1FFu) << 21;
  packed |= (q1 & 0x1FFu) << 12;
  packed |= (q2 & 0x1FFu) << 3;
  // Bits [2..0] remain 0
  return packed;
}

// Decompress a 32-bit packed quaternion back to a unit quaternion
inline UnitQuat decompress_quat(CompressedQuat packed) {
  UnitQuat out{0.0f, 0.0f, 0.0f, 0.0f};

  const u32 idx = (packed >> 30) & 0x3u;
  const u32 q0 = (packed >> 21) & 0x1FFu;
  const u32 q1 = (packed >> 12) & 0x1FFu;
  const u32 q2 = (packed >> 3) & 0x1FFu;

  // Dequantize back to [-1/sqrt(2), +1/sqrt(2)]
  const float inv_sqrt2 = 0.70710678118654752440f; // 1/sqrt(2)
  const float lo = -inv_sqrt2;
  const float hi = inv_sqrt2;
  const float range = hi - lo; // == sqrt(2)

  auto dequant9 = [&](u32 q) -> float {
    float t = static_cast<float>(q) / 511.0f; // [0,1]
    return t * range + lo;                    // [-inv_sqrt2, +inv_sqrt2]
  };

  float comps[3] = {dequant9(q0), dequant9(q1), dequant9(q2)};

  // Place back into the quaternion (x,y,z,w order), skipping idx
  u32 k = 0;
  for (u32 i = 0; i < 4; ++i) {
    if (i == idx)
      continue;
    out[i] = comps[k++];
  }

  // Reconstruct the missing largest component (non-negative by construction)
  float three_sum = 0.0f;
  for (u32 i = 0; i < 4; ++i) {
    if (i == idx)
      continue;
    three_sum += out[i] * out[i];
  }
  float missing = 1.0f - three_sum;
  if (missing < 0.0f)
    missing = 0.0f; // guard numeric drift
  missing = std::sqrt(missing);
  out[idx] = missing;

  // Optional re-normalization to counter round-off
  const float n2 =
      out[0] * out[0] + out[1] * out[1] + out[2] * out[2] + out[3] * out[3];
  if (n2 > 0.0f && std::abs(n2 - 1.0f) > 1e-5f) {
    const float invn = 1.0f / std::sqrt(n2);
    out[0] *= invn;
    out[1] *= invn;
    out[2] *= invn;
    out[3] *= invn;
  }

  return out;
}

// Space/zone identifier used for transform interpretation
// Space/zone identifier used for transform interpretation (Fixed-only)
struct CompressedTransformSpaceComponent {
  u32 zone;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(TransformComponent);
N2_STATIC_ASSERT_FIXED(CompressedTransformComponent);
N2_STATIC_ASSERT_FIXED(CompressedTransformSpaceComponent);

// Ephemeral advisory transform (client->server only)
// using ProposedTransformComponent = TransformComponent;
