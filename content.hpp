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

struct MsgCreateEntities {
  // DYN
  DynamicArray<u32, EntityID> new_entities;
};

struct MsgDeleteEntities {
  // DYN
  DynamicArray<u32, EntityID> to_delete;
};

// ==============================================================================
// Component modification
// Adds a component to an entity, given by entity_id
// If the entity id is missing, create entity first.
// If the component ID is negative, that means the component content should be
// ignored and the component removed from the entity

using ContentSize = u32;

struct Modification {
  EntityID id;
  OpaqueBytes content;
};

struct MsgModifyComponent {
  ComponentTypeID type;

  // this MUST be the size of the following vector
  // In order for clients who do not understand the type to skip the message
  ContentSize message_size;
  DynamicArray<u32, Modification> modifications;
};

struct MsgRemoveComponent {
  ComponentTypeID type;
  DynamicArray<u32, EntityID> entities;
};

// ==============================================================================
// Resource modification

struct MsgModifyResource {
  ResourceTypeID type;

  // this MUST be the size of the following opaque bytes
  // In order for clients who do not understand the type to skip the message
  ContentSize message_size;
  OpaqueBytes bytes;
};

struct MsgRemoveResource {
  ResourceTypeID type;
};

// ==============================================================================
// Asset modification

// Register an asset for use
struct MsgModifyAsset {
  AssetTypeID type;
  AssetID asset_id;

  // this MUST be the size of the following opaque bytes
  // In order for clients who do not understand the type to skip the message
  ContentSize message_size;
  OpaqueBytes bytes;
};

// Assets may be removed, but, in this case, any handle to an asset MUST be kept
// alive.
struct MsgRemoveAsset {
  AssetTypeID type;
  AssetID asset_id;
};

// ============================================================================================

using ContentMessage =
    MatchGroup<ContentMessageID, MsgCreateEntities, MsgDeleteEntities,
               MsgModifyComponent, MsgRemoveComponent, MsgModifyResource,
               MsgRemoveResource, MsgModifyAsset, MsgRemoveAsset>;
