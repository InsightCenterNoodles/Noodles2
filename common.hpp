#pragma once

#include <array>

// All types parsed as c-style, IEEE, little-endian

using i8 = unsigned char;
using i16 = unsigned short;
using i32 = unsigned int;
using i64 = int64_t;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = uint32_t;
using u64 = uint64_t;

// Use NULL to indicate end
template <size_t Count> using FixedShortString = std::array<char, Count>;

// All messages in this spec have static and dynamic content. the static must be
// parsed first, then dynamic. the DYN comment marker is the delimiter

// Try to align things to 8 byte boundaries
// padd all messages and structs to 8 bytes? not sure on this one

// match an integer to a class, dynamic
template <class ID, class... Ts> struct MatchGroup {
  ID type;
};

// DO NOT STORE THIS CLASS. this is a marker that readers must read dynamically
// sized content
template <class SizeType, class T> class DynamicArray {
  SizeType count;
};

// DO NOT STORE THIS CLASS. this is a marker that readers must read dynamically
// sized content
template <class SizeType, class K, class V> class DynamicMap {
  SizeType count;
};

template <class SizeType> using DynamicByteArray = DynamicArray<SizeType, u8>;

/// @brief A utf8 string
/// @tparam SizeType
template <class SizeType> using DynamicString = DynamicArray<SizeType, u8>;

/// @brief A blob of bytes; size and content are determined elsewhere.
struct OpaqueBytes {};

/// Denotes that an object may or may not exist, based on some other documented
/// discriminant
template <class T> struct DynamicOptional {};

// =============================================

using EntityID = u32;

// An identifier for a component type
// Note: This is a signed integer!
using ComponentTypeID = u32;

// An identifier for a resource type
using ResourceTypeID = u32;

// A specific asset type
using AssetTypeID = u32;

// An identifier for a specific asset. Different asset types are considered
// different namespaces for these IDs
using AssetID = u32;

struct AnAsset {
  AssetTypeID type;
  AssetID id;
};

using Vec3 = std::array<float, 3>;

// A rotation quaternion. The basis coefficients are encoded in xyz, with the
// scalar in w This MUST be a normalized unit quaternion!
using UnitQuat = std::array<float, 4>;

using UUID = std::array<u32, 4>;