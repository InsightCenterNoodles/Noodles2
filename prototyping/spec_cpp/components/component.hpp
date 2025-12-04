#pragma once

#include "../common.hpp"

// Components SHOULD be fixed only. Avoid dynamics.

// Canonical component type identifiers
enum class ComponentType : u32 {
  NAME = 1,

  TRANSFORM = 4,
  COMPRESSED_TRANSFORM = 5,
  COMPRESSED_TRANSFORM_SPACE = 6,

  INTERACTIVE = 8,
  CAN_TRANSLATE = 9,
  CAN_ROTATE = 10,
  CAN_SCALE = 11,

  CAN_SELECT = 12,
  IS_SELECTED = 13,

  ACTIVATE = 15,

  VISIBLE = 20,

  BILLBOARD = 21,
  TEXT = 22,

  CHILD_OF = 25,

  RENDER_LAYER = 30,

  MESH = 50,
  MATERIAL = 51,
  ENVIRONMENT = 52,

  DIRECTIONAL_LIGHT = 60,
  POINT_LIGHT = 61,
  SPOT_LIGHT = 62,
};
