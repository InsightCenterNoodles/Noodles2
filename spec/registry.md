Extensions define namespaces for components and assets. Each is assigned a 16-bit identifier that scopes type IDs on the wire. Only extensions advertised by both peers during the preamble should appear in subsequent messages.

| Extension | Identifier | Description |
|-----------|------------|-------------|
| Core      | 0x0000     | Base assets and components required for interoperability. |

Notes:
- Identifiers are little-endian on the wire.
- Submit new extension IDs alongside a short description and contact so they can be recorded here.
- Components should have a "Component" postfix. Assets should have an "Asset" postfix.