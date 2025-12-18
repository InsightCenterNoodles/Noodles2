# NOODLES2

**NLR Object Oriented Data Layout and Exploration System 2**

NOODLES2 defines a protocol for collaborative, distributed visualization of structured simulation data. It is based on the Entity-Component-System (ECS) architectural model and uses a compact, custom little-endian binary message format, typically over Websockets. The protocol is intended for real-time synchronization of 2D and 3D visual representations across networked clients.

---

## Protocol Shape

- ECS-centered: entities/assets use 64-bit IDs (bit 63 reserved for patch-local placeholders in proposals), with components as typed data blobs and assets as payloads referenced by components.
- Binary-first: layouts defined in `.jaw` schemas; little-endian, size-prefixed arrays; no optional fields aside from explicit variants.
- Directed links: every connection has an Upstream (authority) and one or more Downstreams (proposer/consumer). Authority can be negotiated at handshake via `authority_request`.
- Extensions: each extension owns component/asset ID space under a 16-bit namespace; only mutually advertised extensions should be used on a connection.
    - Extension evolution is done through the deprecation and addition of a new extension. 
    - Old extension versions can be supported alongside new.

## Wire Format & Framing
- Messages are in binary, little-endian, IEEE-754. 
- Messages are described by the Jaw format. 
- For frame-based transports, there is one message per transport frame. For byte-oriented transports, no prefix is required, messages are simply parsed as the next in the sequence as given by the connection flow.

## Connection Flow

- All connections start in the Preamble state, until Upstream and Downstream assignments are determined.
    - Upon connection open the initiator of the connection is the Client and the receiver is the Server. 
    - Only PreMessage may be sent during this time.
    - The Client sends an `Introduction` message, which exchanges protocol version, capabilities (extension IDs).
    - The Server validates and compares protocol version and capabilities, and responds with a `IntroductionReply`.
    - The Server is termed the Upstream (the authority) and the client is the Downstream.
        - If the introduction includes the authority swap flag, the Client states that it wishes to be the Upstream in this relationship. If the Server agrees, it will send the Introduction, as if the connection sequence was reversed. If it does not agree, the connection is closed.
    - Upstream and Downstream determined, preamble over.
        - If the Client sends a `Probe` message, it indicates they wish to obtain details, but not join
          the current session. A `ProbeReply` is sent by the Server and the connection is closed.
- Once Upstream and Downstream are assigned, the connection moves to the Main Sequence state.
    - Messages are encoded as a series of `MainSequenceMessage` variants.
    - Upstream sends initial state of the ECS world to Downstream.
    - Delta messages proceed afterward at a rate of the Upstream's choosing.
        - It is recommended that these are on the order of 1 Hz.
    - The connection may be closed at any time
- Additional:
    - The `protocol_version` corresponds to a version of this specification, but does not cover extensions.
        - If the protocol version does not match on introduction, Clients and Servers are recommended to disconnect. 
    - Capability gating: during connection, each side must only send component/asset types contained in the shared extension set. No 'unknown' extension content, or 'unknown' content inside extensions can be sent.
    - Nodes may disconnect at any time should this occur.
    - During Preamble, Nodes may choose how long to wait for introduction or reply, closing the connection if a message is not resceived in this time. It is recommended that this timeout be on the order of 10 seconds.
    - During Main Sequence, there should no timeout, as state may not change significantly during use.

## ECS and World
- Coordinate system is modeled after the OpenGL/GLTF coordinate system.

## Transactions and Authority

- Downstream may ask to change the ECS world. Downstream proposes, Upstream decides. Proposals use `ProposedChangeTransaction` carrying the same `ContentMessage` variants as authoritative `Transaction`. New entities/assets in a proposal MUST use patch-local IDs with bit 63 set; canonical IDs keep bit 63 clear. This lets a single proposal mix references to existing and newly created content; Upstream rejects unknown canonical IDs or patch-local IDs outside proposals.
- Proposals are processed in strict arrival order. On acceptance, Upstream sends `ProposedTransactionReply` (code 0) with remap tables from patch-local → canonical entities/assets, followed by a canonical `Transaction` broadcast that includes the applied changes and any cascade effects. Non-zero codes are rejections (1 denied, 2 resource exhausted, 3 malformed/disallowed, >=256 application-specific). No error message payload is returned.
- Asset payloads in proposals must be inline buffers (no URLs or OOB) from Downstream to Upstream. Asset deletion proposals are rejected.

## RPC and Interaction

- Entities can advertise callable endpoints with the `RPCEndpointComponent`. Arguments and results are carried as `Any` values.
- `RPCInvoke` references the target entity; `RPCReply` returns `RPCComplete`, `RPCStreamResult`, or `RPCError` with a UTF-8 message.
- Interaction-related components (`CanTranslate`, `CanRotate`, `CanScale`, `Activate`) describe affordances; they are pure data and require application-level semantics.

## Assets and OOB Transfer

- Assets are addressed by `AssetTypeID` + `AssetID`. Large, immutable payloads can flow over optional out-of-band (OOB) asset channels (`OOBAssetInfo` in the preamble) to avoid blocking the main communication socket.
- `BufferView` references a `BufferAssetID`; higher-level assets (mesh, image, environment, material) reference buffer-backed views.
- OOB support is provided through a number of channels (in this case, websockets), where async asset requests can be made. These may be kept open as long as the main channel is open. Content is in the same binary format as the main channel, but with a restricted set of messages.
    - Connections must not be maintained with a node that is not connected on the main channel.
    - A request is made by a `OOBRequest`, and serviced by a single `OOBReply` response.
    - A buffer asset may contain an OOBAssetID, or blob of bytes, that can be requested from Upstream.
    - OOB errors include the requested `asset_id` plus a UTF-8 reason string.

## Animation and Deformation

- Core includes an `AnimationAsset` (samplers/channels) plus `AnimatorComponent` for layered playback.
- Mesh assets now describe joint/weight attributes for skinning and optional morph targets; `SkinComponent` supplies joint palettes and inverse bind matrices.
- `MorphWeightsComponent` carries per-entity weights; animation channels may target specific weights by index.

## Extensions

The specification is designed for expandability and modularity through Extensions. Extensions define a collection of components, assets, and semantics with optional dependencies on other Extensions.

- During handshake, each side advertises supported extensions. Only the intersection should be used on the link.
- Extensions are registered as 16-bit namespace IDs for components and assets. The registry is tracked in `spec/registry.md`.
- The Core extension (0x0000) defines the base component and asset types in `spec/core`. All implementations must support this core extension.


## Status and Roadmap

- Beta-stage; schemas may still change. Systems (runtime behavior), richer capability negotiation, and transport options (e.g., libp2p) remain on the roadmap.
- Feedback and implementation notes are welcome; keep the `.jaw` schemas and markdown docs in sync as behavior evolves.
