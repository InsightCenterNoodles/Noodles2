# Core Extension


## Assets

|Name      | Identifier | Description |
|----------|------------| ----------- |
| Buffer        | 0x0000     | A blob of bytes, used in other assets for providing byte-data |
| Environment   | 0x0001     | An environment map |
| Image         | 0x0002     | A decoded image, for use in texturing |
| Mesh          | 0x0003     | Geometry data |
| PBRMaterial   | 0x0004     | A PBR material |


## Components
|Name      | Identifier | Description |
|----------|------------| ----------- |
| Global        | 0x0000     | Indicates that this entity has components that should be considered global. |
| RPCEndpoint   | 0x0001     | An RPC call endpoint. |
| Billboard     | 0x0002     | The entity should transform to always face the user. |
| Environment   | 0x0003     | An environment skybox/light |
| Instance      | 0x0004     | Explicit instancing |
| Interactive   | 0x0005     | Can interact with entity |
| CanTranslate  | 0x0006     | Can translate or move the entity |
| CanRotate     | 0x0007     | Can rotate the entity |
| CanScale      | 0x0008     | Can scale the entity |
| Activate      | 0x0009     | Can the entity be "activated" or "clicked" |
| DirectionalLight | 0x0010  | A parallel ray light source |
| PointLight    | 0x0011     | A point light source |
| SpotLight     | 0x0012     | A spot light source |
| PBRMaterial   | 0x0013     | Attach a material to an entity |
| Mesh          | 0x0014     | Attach a mesh to an entity |
| Name          | 0x0015     | Provide a name to an entity |
| ChildOf       | 0x0016     | Establish a parent-child entity relationship |
| CanSelect     | 0x0017     | Can the entity be selected |
| IsSelected    | 0x0018     | Attached if the entity is selected |
| Text          | 0x0019     | 3D text rendering |
| Transform     | 0x0020     | Local transformation |
| Visibility    | 0x0021     | Scene visibility |
