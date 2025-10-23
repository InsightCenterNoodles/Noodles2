#include "../common.hpp"

struct QuantizationSpace
{
    u32 index;
    Vec3 min;
    Vec3 max;
};

struct CompressedTransformResource
{
    DynamicArray<u32, QuantizationSpace> spaces;
};