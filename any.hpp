#pragma once

#include "common.hpp"

enum class AnyType : u8 {
  UNDEF = 0,
  BOOL = 1,
  INT = 2,
  FLOAT = 3,
  STRING = 4,
  ARRAY = 5,
  MAP = 6,
  BYTES = 7,
  VECTOR = 8,
  UNIT_QUAT = 9,
  UUID = 10,
  COMPONENT_TYPE = 11,
  ASSET_TYPE = 12,
  AN_ASSET = 13,
  RESOURCE_TYPE = 14,
  ENTITY = 15,
};

struct Any;
struct Any
    : MatchGroup<AnyType, //
                 void, i64, double, DynamicString<u64>, DynamicArray<u32, Any>,
                 DynamicMap<u32, Any, Any>, DynamicByteArray<u64>, Vec3,
                 UnitQuat, UUID, ComponentTypeID, AssetID, AnAsset,
                 ResourceTypeID, EntityID> {};