#pragma once

#include "../common.hpp"

// Canonical component type identifiers
enum class ComponentType : u32 {
  NAME = 1,

  TRANSFORM = 4,
  COMPRESSED_TRANSFORM = 5,
  COMPRESSED_TRANSFORM_SPACE = 6,
  // PROPOSED_TRANSFORM = 7,

  INTERACTIVE = 8,
  CAN_TRANSLATE = 9,
  CAN_ROTATE = 10,
  CAN_SCALE = 11,

  ACTIVATE = 15,

  VISIBLE = 20,

  CHILD_OF = 25,

  MESH = 50,
  MATERIAL = 51,
};

// RPC endpoint identifier used by activation logic
using RpcEndpointID = u32;
