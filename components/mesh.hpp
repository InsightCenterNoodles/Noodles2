#pragma once

#include "../asset/asset.hpp"

// Mesh to render for this entity (Fixed-only)
struct MeshComponent {
  MeshAssetID mesh;
};

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(MeshComponent);
