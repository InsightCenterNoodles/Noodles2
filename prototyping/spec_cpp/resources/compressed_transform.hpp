#include "../common.hpp"

// QuantizationSpace is fixed-only
struct QuantizationSpace {
  u32 index;
  Vec3 min;
  Vec3 max;
};

// CompressedTransformResource holds a dynamic list of spaces
struct CompressedTransformResourceDyn {
  DynamicArray<u32, QuantizationSpace> spaces;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(QuantizationSpace);
