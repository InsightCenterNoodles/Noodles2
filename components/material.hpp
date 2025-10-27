#pragma once

#include "../asset/asset.hpp"

// Materials to use for rendering; if more than mesh patches, list is repeated
// as prefix (Fixed-only)
struct StdMaterialComponent {
  StdMaterialAssetID materials;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(StdMaterialComponent);
