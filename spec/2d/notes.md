# Prototyping for 2D Extension

- Camera / projection: 
    - There’s no component for projection in our 3D spec as there is no point; clients render how they want to render; cameras make no sense for immersive, for example.
    - But in 2D, things are now more strict. Clients have to assume an orthographic camera or invent rules. 
    - A Camera2DComponent: viewport, zoom, pixel scale, world units per pixel
- Render ordering: 
    - 2D needs explicit layering (z‑order, sorting groups).
    - Order is implicit via z or unspecified.
    - RenderLayerComponent or SortKeyComponent would remove ambiguity.
- Sprites / UV rects: 
    - SpriteComponent could reference ImageAsset plus UV rect, size, pivot, flip, and maybe blend mode.
- Tilemaps: 
    - A 2D‑native asset for tile layers, chunking, and atlas references would avoid huge meshes
- 2D materials: 
    - PBRMaterialAsset is overkill for 2D
    - UnlitMaterialAsset or SpriteMaterialAsset (tint, opacity, blend mode)
- Picking / interaction: 
    - 2D needs hit shapes (rect, circle, polygon)
- Text layout: 
    TextComponent only has color + height + content. Need font, alignment, wrapping, and max width/height.
