#pragma once

#include "../asset/asset.hpp"

// Materials to use for rendering; if more than mesh patches, list is repeated
// as prefix
struct StdMaterialComponent {
  StdMaterialAssetID materials;
};
