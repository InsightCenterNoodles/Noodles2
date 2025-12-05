# NOODLES2

**NLR Object Oriented Data Layout and Exploration System 2**

NOODLES2 defines a protocol for collaborative, distributed visualization of structured simulation data. It is based on the Entity-Component-System (ECS) architectural model and uses a compact, custom little-endian binary message format, typically over Websockets. The protocol is intended for real-time synchronization of 2D and 3D visual representations across networked clients.

---

## Protocol Shape

- ECS-centered: entities (u32 IDs), components (typed data blobs), and assets (immutable payloads referenced by components).
- Binary-first: layouts defined in `.jaw` schemas; little-endian, size-prefixed arrays; no optional fields aside from explicit variants.
- Directed links: every connection has an Upstream (authority) and Downstream (proposer/consumer). Authority can be negotiated at handshake via `authority_request`.
- Extensions: each extension owns component/asset ID space under a 16-bit namespace; only mutually advertised extensions should be used on a connection.

## Connection Flow

- Preamble: `Introduction` (or `Probe`) exchanges protocol version, capabilities (extension IDs), and optional out-of-band (OOB) asset channel hints. `authority_request` lets a client ask to be treated as the authority, swapping roles.
- Main sequence: all stateful traffic after the handshake. Messages are encoded as `MainSequenceMessage` variants.
- Capability gating: during connection, each side must only send component/asset types contained in the shared extension set.

## Transactions and Authority

- Downstream proposes, Upstream decides. Proposals use `ProposedChangeTransaction` with two buckets: `modify_content` (operates only on canonical world IDs; for modifying/deleting components/entities/assets) and `create_content` (transaction-local IDs for new entities/assets; remapped on accept). `create_content` cannot reference existing world IDs, and `modify_content` cannot reference IDs introduced in `create_content`.
- Proposals are processed in strict arrival order. On acceptance, Upstream sends `ProposedTransactionReply` (code 0, with remap tables for created entities/assets) followed by a canonical `Transaction` broadcast that includes the applied changes and any cascade effects. Non-zero codes are rejected (1 denied, 2 resource exhausted, 3 malformed/disallowed, >=256 application-specific). No error message payload is returned.
- Asset payloads in proposals must be inline buffers (no URLs or OOB) from Downstream to Upstream. Asset deletion proposals are rejected. Content variants allowed in `create_content`: create entities, modify/delete components, delete entities, modify asset (interpreted as create asset for local IDs). `modify_content` uses canonical IDs and excludes `ContentDeleteAsset`.

## RPC and Interaction

- Entities can advertise callable endpoints with the `RPCEndpointComponent`. Arguments and results are carried as `Any` values.
- `RPCInvoke` references the target entity; `RPCReply` returns `RPCComplete`, `RPCStreamResult`, or `RPCError` with a UTF-8 message.
- Interaction-related components (`CanTranslate`, `CanRotate`, `CanScale`, `Activate`) describe affordances; they are pure data and require application-level semantics.

## Assets and OOB Transfer

- Assets are addressed by `AssetTypeID` + `AssetID`. Large, immutable payloads can flow over optional out-of-band asset channels (`OOBAssetInfo` in the preamble) to avoid blocking the main communication socket.
- `BufferView` references a `BufferAssetID`; higher-level assets (mesh, image, environment, material) reference buffer-backed views.
- OOB errors include the requested `asset_id` plus a UTF-8 reason string.

## Extensions

The specification is designed for expandability and modularity through Extensions. Extensions define a collection of components, assets, and semantics with optional dependencies on other Extensions.

- During handshake, each side advertises supported extensions. Only the intersection should be used on the link.
- Extensions are registered as 16-bit namespace IDs for components and assets. The registry is tracked in `spec/registry.md`.
- The Core extension (0x0000) defines the base component and asset types in `spec/core`. All implementations must support this core extension.


## Status and Roadmap

- Beta-stage; schemas may still change. Systems (runtime behavior), richer capability negotiation, and transport options (e.g., libp2p) remain on the roadmap.
- Feedback and implementation notes are welcome; keep the `.jaw` schemas and markdown docs in sync as behavior evolves.
