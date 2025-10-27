#pragma once

#include "common.hpp"

enum class ContentMessageID {
  CREATE_ENTITIES = 0,
  DELETE_ENTITIES = 1,

  MODIFY_COMPONENT = 2,
  DELETE_COMPONENT = 3,

  MODIFY_RESOURCE = 4,
  DELETE_RESOURCE = 5,

  MODIFY_ASSET = 6,
  DELETE_ASSET = 7,
};

// ==============================================================================
// Entity message

/// Create entities (Dyn-only): a list of new entity IDs
struct MsgCreateEntitiesDyn {
  DynamicArray<u32, EntityID> new_entities;
};

/// Delete entities (Dyn-only): a list of entity IDs to remove
struct MsgDeleteEntitiesDyn {
  DynamicArray<u32, EntityID> to_delete;
};

// ==============================================================================
// Component modification
// Adds or replaces a component on an entity, given by entity_id.
// If the entity id is missing, the receiver may create the entity first.
// Removal is explicit via DELETE_COMPONENT; there are no negative-ID sentinels.

using ContentSize = u32;

/// Component modification element within MsgModifyComponentDyn
/// Fixed: target entity; Dyn: opaque component bytes interpreted by the receiver
struct Modification {
  EntityID id;
  N2_NO_UNIQUE OpaqueBytes content;
};

/// Modify component: Fixed header then Dyn modifications
/// - Fixed carries the ComponentTypeID and a byte size bounding the Dyn block
///   so unknown component types can be skipped safely.
struct MsgModifyComponentFixed {
  ComponentTypeID type;

  // this MUST be the size of the following vector
  // In order for clients who do not understand the type to skip the message
  ContentSize message_size;
};

struct MsgModifyComponentDyn {
  DynamicArray<u32, Modification> modifications;
};

/// Remove component: Fixed type then Dyn entity list
struct MsgRemoveComponentFixed {
  ComponentTypeID type;
};

struct MsgRemoveComponentDyn {
  DynamicArray<u32, EntityID> entities;
};

// ==============================================================================
// Resource modification

/// Modify resource: Fixed header then Dyn bytes (opaque)
/// - Fixed carries a byte size bounding the Dyn block so unknown types can be
///   skipped safely.
struct MsgModifyResourceFixed {
  ResourceTypeID type;

  // this MUST be the size of the following opaque bytes
  // In order for clients who do not understand the type to skip the message
  ContentSize message_size;
};

struct MsgModifyResourceDyn {
  N2_NO_UNIQUE OpaqueBytes bytes;
};

/// Remove resource (Fixed-only)
struct MsgRemoveResourceFixed {
  ResourceTypeID type;
};

// ==============================================================================
// Asset modification

// Register an asset for use
/// Modify asset: Fixed header then Dyn bytes (opaque)
/// - Fixed carries a byte size bounding the Dyn block so unknown types can be
///   skipped safely.
struct MsgModifyAssetFixed {
  AssetTypeID type;
  AssetID asset_id;

  // this MUST be the size of the following opaque bytes
  // In order for clients who do not understand the type to skip the message
  ContentSize message_size;
};

struct MsgModifyAssetDyn {
  N2_NO_UNIQUE OpaqueBytes bytes;
};

// Assets may be removed, but, in this case, any handle to an asset MUST be kept
// alive.
/// Remove asset (Fixed-only)
struct MsgRemoveAssetFixed {
  AssetTypeID type;
  AssetID asset_id;
};

// ============================================================================================

// Content message union is Dyn at the outer layer; the transport that wraps
// ContentMessage must carry the corresponding Fixed parts as needed.
using ContentMessage = DynamicTaggedUnion<
    ContentMessageID,
    // CREATE_ENTITIES, DELETE_ENTITIES
    Case<ContentMessageID, ContentMessageID::CREATE_ENTITIES, MsgCreateEntitiesDyn>,
    Case<ContentMessageID, ContentMessageID::DELETE_ENTITIES, MsgDeleteEntitiesDyn>,
    // MODIFY_COMPONENT, DELETE_COMPONENT
    Case<ContentMessageID, ContentMessageID::MODIFY_COMPONENT, MsgModifyComponentDyn>,
    Case<ContentMessageID, ContentMessageID::DELETE_COMPONENT, MsgRemoveComponentDyn>,
    // MODIFY_RESOURCE, DELETE_RESOURCE
    Case<ContentMessageID, ContentMessageID::MODIFY_RESOURCE, MsgModifyResourceDyn>,
    Case<ContentMessageID, ContentMessageID::DELETE_RESOURCE, MsgRemoveResourceFixed>,
    // MODIFY_ASSET, DELETE_ASSET
    Case<ContentMessageID, ContentMessageID::MODIFY_ASSET, MsgModifyAssetDyn>,
    Case<ContentMessageID, ContentMessageID::DELETE_ASSET, MsgRemoveAssetFixed>>;

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(MsgModifyComponentFixed);
N2_STATIC_ASSERT_FIXED(MsgRemoveComponentFixed);
N2_STATIC_ASSERT_FIXED(MsgModifyResourceFixed);
N2_STATIC_ASSERT_FIXED(MsgRemoveResourceFixed);
N2_STATIC_ASSERT_FIXED(MsgModifyAssetFixed);
N2_STATIC_ASSERT_FIXED(MsgRemoveAssetFixed);
