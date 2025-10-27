#pragma once

#include <array>

// Attribute helper: eliminate storage for empty members where supported
#ifndef N2_NO_UNIQUE
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(no_unique_address)
#define N2_NO_UNIQUE [[no_unique_address]]
#else
#define N2_NO_UNIQUE
#endif
#else
#define N2_NO_UNIQUE
#endif
#endif

// Wire primitives and endianness
// - All fixed-width integers are little-endian on the wire.
// - Floating-point values are IEEE-754 little-endian.
// - No implicit padding is inserted on the wire; only the fields declared
//   in headers are serialized in the order written.

// Fixed/Dyn helpers
#ifndef N2_STATIC_ASSERT_FIXED
#define N2_STATIC_ASSERT_FIXED(T)                                              \
  static_assert(std::is_trivially_copyable<T>::value,                          \
                #T " must be trivially copyable (Fixed)")
#endif

// Numeric aliases
// Note: Prefer u8/u16/u32/u64 for unsigned integers. The i64 alias is provided
// for signed 64-bit integers.
using i64 = int64_t;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = uint32_t;
using u64 = uint64_t;

// Fixed-size, null-terminated short string
// - UTF-8 or ASCII content; terminates with '\0' if shorter than Count.
// - Unused bytes after the first '\0' MUST be zero.
// - No dynamic length; consumers MUST scan up to the first '\0' or Count.
template <size_t Count> using FixedShortString = std::array<char, Count>;

// Wire layout convention
// ----------------------
// Each message/struct may be split into two parts using a naming convention:
//   - FooFixed: memcpy-safe, trivially copyable fields. Little-endian, C-style.
//   - FooDyn: parsed sequentially after FooFixed; contains
//   dynamic/variable-size
//             members (e.g., DynamicArray, DynamicMap, DynamicOptional,
//             DynamicTaggedUnion, OpaqueBytes, strings, etc.).
//
// The wire layout is "[FooFixed][FooDyn…]" when both exist. Either part may be
// absent: some types are Fixed-only (no Dyn), others are Dyn-only (no Fixed).
//
// Notes:
// - FooFixed must not contain pointers or variable-length members.
// - FooDyn may contain dynamic constructs and is read field-by-field.
// - We do not yet include any size prefix (e.g., dyn_size) for FooDyn.
// - The previous "DYN" in-file comment delimiter is deprecated and removed in
//   favor of the explicit FooFixed/FooDyn types.

// Alignment
// - On-wire layout is tightly packed; do not add implicit padding between
//   fields. Natural alignment concerns apply only to in-memory views.

// Tagged union helpers
// Prefer these over DynamicMatchGroup for explicit, self-documenting mappings.
// Usage:
//   using X = DynamicTaggedUnion<MyTag,
//       Case<MyTag, MyTag::A, PayloadA>,
//       Case<MyTag, MyTag::B, PayloadB>>;
template <class Tag, Tag V, class T> struct Case {};

template <class Tag, class... Cases> struct DynamicTaggedUnion {
  Tag type; // discriminant read from the wire first
};

// Deprecated alias: older notation that relied on positional mapping
// Keep for compatibility in headers, but prefer DynamicTaggedUnion + Case.
template <class ID, class... Ts> struct DynamicMatchGroup {
  ID type;
};

// DO NOT STORE THIS CLASS. this is a marker that readers must read dynamically
// sized content
// DynamicArray: DO NOT STORE. Marker indicating a sequence of T elements.
// Wire format: [count: SizeType][element_0][element_1]...[element_{count-1}].
// Elements are serialized back-to-back in declared order.
template <class SizeType, class T> class DynamicArray {
  SizeType count;
};

// DO NOT STORE THIS CLASS. this is a marker that readers must read dynamically
// sized content
// DynamicMap: DO NOT STORE. Marker indicating a sequence of (K,V) pairs.
// Wire format: [count: SizeType][K0][V0][K1][V1]... repeated count times.
template <class SizeType, class K, class V> class DynamicMap {
  SizeType count;
};

template <class SizeType> using DynamicByteArray = DynamicArray<SizeType, u8>;

/// @brief A utf8 string (dynamic)
/// Wire format: [byte_count: SizeType][bytes...]; no terminator; bytes are
/// raw UTF-8 code units (no BOM).
template <class SizeType> using DynamicString = DynamicArray<SizeType, u8>;

/// @brief A blob of bytes; size and content are determined elsewhere.
/// Readers MUST be able to determine the extent from enclosing Fixed fields
/// (e.g., an explicit size) or message framing.
struct OpaqueBytes {};

/// Denotes that an object may or may not exist, based on a documented
/// discriminant outside the object (e.g., a flag bit in a Fixed header).
template <class T> struct DynamicOptional {};

// =============================================

using EntityID = u32;

// An identifier for a component type
// Note: In this reference, IDs are unsigned. Older notes referred to signed
// IDs for deletion sentinels; deletion is now an explicit message.
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
