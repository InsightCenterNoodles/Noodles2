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

// Any: a dynamic, self-describing value (Dyn-only)
// Encoded as an explicit tagged union with named cases
struct Any;
struct Any
    : DynamicTaggedUnion<
          AnyType,
          Case<AnyType, AnyType::UNDEF, void>,
          Case<AnyType, AnyType::BOOL, u8>,
          Case<AnyType, AnyType::INT, i64>,
          Case<AnyType, AnyType::FLOAT, double>,
          Case<AnyType, AnyType::STRING, DynamicString<u64>>,
          Case<AnyType, AnyType::ARRAY, DynamicArray<u32, Any>>,
          Case<AnyType, AnyType::MAP, DynamicMap<u32, Any, Any>>,
          Case<AnyType, AnyType::BYTES, DynamicByteArray<u64>>,
          Case<AnyType, AnyType::VECTOR, Vec3>,
          Case<AnyType, AnyType::UNIT_QUAT, UnitQuat>,
          Case<AnyType, AnyType::UUID, UUID>,
          Case<AnyType, AnyType::COMPONENT_TYPE, ComponentTypeID>,
          Case<AnyType, AnyType::ASSET_TYPE, AssetTypeID>,
          Case<AnyType, AnyType::AN_ASSET, AnAsset>,
          Case<AnyType, AnyType::RESOURCE_TYPE, ResourceTypeID>,
          Case<AnyType, AnyType::ENTITY, EntityID>> {};
