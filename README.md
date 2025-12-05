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

- Preamble: `Introduction` (or `Probe`) exchanges protocol version, capabilities (extension IDs), and optional out-of-band (OOB) asset channel hints. `authority_request` lets a client ask to be treated as the authority.
- Main sequence: all stateful traffic after the handshake. Messages are encoded as `MainSequenceMessage` variants.
- Capability gating: during connection, each side must only send component/asset types contained in the shared extension set.

## Transactions and Authority

- Downstream proposes, Upstream decides. A proposal is a `Transaction` (or `NamedTransaction`) carrying a list of `ContentMessage` items (create/delete entities, modify/delete components, modify/delete assets).
- `TransactionReply.code` = 0 means accepted; non-zero means rejected with an application-defined reason. Transactions are atomic.
- Upstream should rebroadcast the accepted canonical state after applying a transaction, rather than relying on a Downstream echo.

## RPC and Interaction

- Entities can advertise callable endpoints with the `RPCEndpointComponent`. Arguments and results are carried as `Any` values.
- `RPCInvoke` references the target entity; `RPCReply` returns `RPCComplete`, `RPCStreamResult`, or `RPCError` with a UTF-8 message.
- Interaction-related components (`CanTranslate`, `CanRotate`, `CanScale`, `Activate`) describe affordances; they are pure data and require application-level semantics.

## Assets and OOB Transfer

- Assets are addressed by `AssetTypeID` + `AssetID`. Large, immutable payloads should flow over optional out-of-band asset channels (`OOBAssetInfo` in the preamble).
- `BufferView` always references a `BufferAssetID`; higher-level assets (mesh, image, environment, material) reference buffer-backed views.
- OOB errors include the requested `asset_id` plus a UTF-8 reason string.

## Extensions

- Extension IDs are 16-bit namespaces for components and assets. The registry is tracked in `spec/registry.md`.
- The Core extension (0x0000) defines the base component and asset types in `spec/core`.
- During handshake, each side advertises supported extensions. Only the intersection should be used on the link.

## Status and Roadmap

- Beta-stage; schemas may still change. Systems (runtime behavior), richer capability negotiation, and transport options (e.g., libp2p) remain on the roadmap.
- Feedback and implementation notes are welcome; keep the `.jaw` schemas and markdown docs in sync as behavior evolves.
