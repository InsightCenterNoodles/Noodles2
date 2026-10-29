Extensions define namespaces for components and assets. Each 16-bit extension ID
identifies one immutable wire-and-semantics contract. An incompatible revision of
an optional extension receives a new extension ID; Core revisions are covered by
the base protocol version. The intersection advertised by both peers during the
preamble is the shared extension set. Downstream proposals MUST remain within
that set; upstream authoritative state SHOULD remain within it, and receivers
MAY skip unsupported extension payloads.

| Extension | Identifier | Description |
|-----------|------------|-------------|
| Core      | 0x0000     | Base assets and components required for interoperability. |
| Audio     | 0x0001     | Optional buffer-backed spatial audio sources; depends on Core. |

Notes:
- Identifiers are little-endian on the wire.
- Capability lists are sets: order and duplicate entries have no meaning.
- Core (`0x0000`) and every dependency of an advertised extension must also be advertised.
- Each extension specification lists its required extension IDs. Advertising it without all dependencies is malformed introduction data.
- Component and asset identifiers are permanent within an extension. Retired identifiers remain reserved and are never reassigned.
- An unsupported or unnegotiated extension may be skipped. An unknown type identifier inside a negotiated extension is a protocol violation.
- Old and new extension IDs may be advertised together. Producing the same logical component through both versions on one entity is discouraged unless their specifications explicitly define coexistence.
- Submit new extension IDs alongside a short description and contact so they can be recorded here.
- Components should have a "Component" postfix. Assets should have an "Asset" postfix.

Well-known semantic event contracts are registered separately in
`well_known_events.md`; their numeric definition identifiers are scoped to the
registry entity rather than assigned globally.
