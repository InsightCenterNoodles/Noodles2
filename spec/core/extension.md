# Core Extension

The Core extension defines the baseline assets and components required for interoperability. IDs are scoped under extension 0x0000 and should be available on every connection.

## Assets

|Name      | Identifier | Description |
|----------|------------| ----------- |
| Buffer        | 0x0000     | Raw byte storage, referenced by BufferView for higher-level assets. |
| Environment   | 0x0001     | Environment/sky lighting; references image table entries by index. |
| Image         | 0x0002     | Encoded image plus sampler/semantic flags. |
| Mesh          | 0x0003     | Geometry attributes and indices in an uncompressed layout. |
| PBRMaterial   | 0x0004     | PBR surface parameters with optional extended features; texture tables limited to 16 entries. |

Notes:
- `BufferView` references must target `Buffer` assets; higher-level assets reference buffers via views.
- Image/material tables are intentionally small (up to 16 entries) to align with typical sampler limits.


## Components
|Name      | Identifier | Description |
|----------|------------| ----------- |
| Global        | 0x0000     | Marks entities with scene-wide effects that should be prioritized. |
| RPCEndpoint   | 0x0001     | Advertises callable endpoints; argument hints are informational. |
| Billboard     | 0x0002     | Rotates to face the viewer (full face or Y-axis only). |
| Environment   | 0x0003     | Binds an environment asset with intensity/rotation. |
| Instance      | 0x0004     | Describes instancing buffers and quantization ranges. |
| Interactive   | 0x0005     | Marks entities as interactive. |
| CanTranslate  | 0x0006     | Translation affordance with hard per-axis limits. |
| CanRotate     | 0x0007     | Rotation affordance with hard per-axis limits. |
| CanScale      | 0x0008     | Scale affordance with hard per-axis limits. |
| Activate      | 0x0009     | Activation targets entities (with RPCEndpoint) to invoke. |
| DirectionalLight | 0x000A  | Directional light source. |
| PointLight    | 0x000B     | Point light source. |
| SpotLight     | 0x000C     | Spot light source. |
| PBRMaterial   | 0x000D     | References a material asset. |
| Mesh          | 0x000E     | References a mesh asset. |
| Name          | 0x000F     | UTF-8 display name. |
| ChildOf       | 0x0010     | Parent-child relationship. |
| CanSelect     | 0x0011     | Entity is selectable. |
| IsSelected    | 0x0012     | Selection state with outline color. |
| Text          | 0x0013     | 3D text with color and height. |
| Transform     | 0x0014     | Local transform (position/rotation/scale). |
| Visibility    | 0x0015     | Visibility state (inherit/visible/invisible). |

Notes:
- Component payloads are complete replacements; partial updates are not defined.
- `Global` marks entities whose effects may be scene-wide. Downstreams may prioritize their synchronization/UI.
- `RPCEndpoint` exposes callable entry points; arguments/results use `Any` encoding.
