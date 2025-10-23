#pragma once

#include "../common.hpp"

// Assign a non-unique text label to an entity
struct NameComponent {
  // DYN
  DynamicString<u32> name;
};
