# NOODLES2

**NLR Object Oriented Data Layout and Exploration System 2**

NOODLES2 defines a protocol for collaborative, distributed visualization of structured simulation data. It is based on the Entity-Component-System (ECS) architectural model and uses a compact, custom little-endian binary message format, typically over WebSockets. The protocol is intended for real-time synchronization of 2D and 3D visual representations across networked clients.

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHOULD**, **SHOULD NOT**,
**MAY**, and **OPTIONAL** are to be interpreted as described by RFC 2119 and
RFC 8174 when, and only when, they appear in all capitals.

---

## Protocol Shape

- ECS-centered: entities/assets use 64-bit IDs (bit 63 reserved for patch-local placeholders in proposals), with components as typed data blobs and assets as payloads referenced by components.
- Binary-first: layouts defined in `.jaw` schemas; little-endian, size-prefixed arrays; no optional fields aside from explicit variants.
- Directed links: every connection has an Upstream (authority) and one or more Downstreams (proposer/consumer). Authority can be negotiated at handshake via `authority_request`.
- Extensions: each extension owns component/asset ID space under a 16-bit namespace. The negotiated shared set governs downstream proposals; upstreams should prefer it for authoritative state, while receivers may skip unknown extension payloads.
    - An extension ID identifies one immutable wire-and-semantics contract. Incompatible optional-extension revisions receive new extension IDs; Core revisions use `protocol_version`.
    - Component and asset identifiers are permanent within an extension; retired identifiers remain reserved.
    - Old and new extension IDs can be supported together.

## Wire Format & Framing
- Messages are in binary, little-endian, IEEE-754. 
- Messages are described by the Jaw format. 
- JAW `bool` occupies one byte; only `0` and `1` are valid encodings.
- For message-oriented transports, one complete NOODLES2 message occupies one
  logical transport message. A logical transport message may itself be
  fragmented by the transport; for example, one WebSocket binary message may
  occupy multiple WebSocket frames. No additional NOODLES2 length prefix is
  used when the transport supplies the logical message boundary.
- Byte-oriented transports use a fixed-width little-endian length prefix before
  each encoded top-level JAW message. The length covers only the JAW payload,
  not the prefix:
    - Main-channel records use a `u32` payload length.
    - OOB-channel records use a `u64` payload length.
  Main and OOB messages are never multiplexed on one channel, so the channel
  binding determines the prefix width without inspecting message content.
- Transport record framing is not part of JAW. A transport implementation
  removes the prefix, if present, and supplies exactly one bounded top-level
  message to the JAW decoder.
- A zero record length, a truncated length or payload, a top-level parse
  failure, unconsumed trailing bytes, or a length exceeding local limits is
  fatal to that connection. Byte-oriented transports do not attempt to
  resynchronize after malformed input.
- Main-channel messages are therefore limited to `u32::MAX` encoded bytes.
  Large payloads should use the OOB asset mechanism, whose content and
  byte-oriented transport records use 64-bit lengths.
- Byte arrays documented as UTF-8 SHOULD contain valid UTF-8. Receivers MAY reject malformed text or replace malformed subsequences when decoding it.
- Nodes MAY impose implementation-defined limits on message sizes, collection lengths, asset sizes, nesting, memory use, and other resource consumption. A node MAY reject content or disconnect when input is malformed, exceeds those limits, or otherwise threatens safe operation.

## Connection Flow

- All connections start in the Preamble state, until Upstream and Downstream assignments are determined.
    - Upon connection open the initiator of the connection is the Client and the receiver is the Server. 
    - Only PreMessage may be sent during this time.
    - The Client sends an `Introduction` message, which exchanges protocol version, capabilities (extension IDs).
    - The Server validates and compares protocol version and capabilities, and responds with an `IntroductionReply`.
    - The Server is termed the Upstream (the authority) and the client is the Downstream.
        - If the introduction includes the authority swap flag, the Client states that it wishes to be the Upstream in this relationship. If the Server agrees, it will send the Introduction, as if the connection sequence was reversed. If it does not agree, the connection is closed.
    - Upstream and Downstream determined, preamble over.
        - If the Client sends a `Probe` message, it indicates they wish to obtain details, but not join
          the current session. A `ProbeReply` is sent by the Server and the connection is closed.
- Once Upstream and Downstream are assigned, the connection moves to the Main Sequence state.
    - Messages are encoded as a series of `MainSequenceMessage` variants.
    - Upstream sends the initial ECS world in zero or more atomic `Transaction` messages, followed by exactly one `InitialStateComplete`. Downstream must not send proposals, RPC invocations, or events before receiving that marker.
    - Subsequent `Transaction` messages carry ordered changes at a rate of the Upstream's choosing. Component payloads are complete replacements; field-level deltas are not defined.
        - It is recommended that these are on the order of 1 Hz.
    - Either node may close the transport at any time. In-flight proposals and RPCs then terminate without replies.
- Additional:
    - The `protocol_version` covers the base protocol, common types, and the mandatory Core extension. Version 0 is development-only and MUST NOT be used by production implementations. Optional extensions are versioned independently through their registered extension IDs.
        - If the protocol versions do not match during introduction, the connection MUST be closed.
    - Capability gating:
        - Both peers MUST advertise the Core extension (`0x0000`).
        - A capability list represents a set: order and duplicate entries have no meaning.
        - A peer MUST advertise every required dependency of each extension it advertises.
        - For Downstream -> Upstream proposals, component/asset types MUST be contained in the shared extension set.
        - For Upstream -> Downstream authoritative state streaming, component/asset types SHOULD be contained in the shared extension set. Receivers MAY ignore unknown extension payloads and continue parsing by skipping their length-prefixed content.
    - Nodes may disconnect at any time should this occur.
    - During Preamble, Nodes may choose how long to wait for an introduction or reply, closing the connection if a message is not received in this time. It is recommended that this timeout be on the order of 10 seconds.
    - During Main Sequence, there should be no inactivity timeout, as state may not change significantly during use.

## ECS and World
- Coordinates are right-handed and mesh front faces use counterclockwise winding, following glTF conventions. Mesh tangent `w` carries the bitangent handedness sign. Scene transforms are intentionally `f32` presentation coordinates for a renderable representation, not a transport for an application's full scientific dataset. Applications map physical/geospatial coordinates into a suitable local scene frame and expose units, source data, or external query facilities through semantic metadata, buffers, or application services.
- Canonical entity IDs are never reused within a session. Deletion removes all components and tombstones the ID; empty entities remain alive until explicitly deleted. A never-seen deletion target may be ignored, while deleting a known tombstone is suspicious and may cause disconnection.
- Each entity has at most one component of each type. Component modification is complete replacement; later repeated updates win, and deleting an absent component is a no-op. Authoritative component modification may implicitly create a never-seen canonical entity but never resurrects a tombstone.
- An entity without a transform has the identity transform. Parent graphs must be acyclic; parents must exist before they are referenced. Ancestor invisibility suppresses descendants, including descendants explicitly marked visible.
- Core transform sampling can smooth low-rate motion updates. Timestamped complete samples retain local history for interpolation or bounded sample-derived extrapolation, with independent position, rotation, and scale markers. A hard `TransformComponent` update resets sampling until a later sample update. Clients may opt out of smoothing work and snap to every newest sample.
- A `GlobalComponent` paired with an environment marks the whole-scene environment. Producers should provide at most one; clients encountering several may choose one. Non-global environments may be spatially blended at client discretion.
- Instance transforms are interleaved and local to their entity, whose world transform is applied afterward. Reduced instances use GPU-friendly UNORM16/SNORM16 groups. Skins require unique live joints; morph weights are finite but need not lie in `[0,1]`.

## Transactions and Authority

- Transactions are atomic. Their content is evaluated in array order, but intermediate states are not observable. Every active reference must resolve when its operation is applied: its target must already be alive or have been created by an earlier operation in the transaction. Forward references are invalid. Reference fields disabled by surrounding flags are ignored.
- Every authoritative transaction carries a non-wrapping `u64` scene generation. Live transactions increase it by exactly one. Initial-snapshot transactions all carry the generation of one consistent snapshot point, repeated by `InitialStateComplete`; Upstream queues later transactions and events until that marker has been sent.
- Downstream may ask to change the ECS world. Downstream proposes, Upstream decides. Proposals use `ProposedChangeTransaction` carrying the same `ContentMessage` variants as authoritative `Transaction`. New entities/assets in a proposal MUST use patch-local IDs with bit 63 set; canonical IDs keep bit 63 clear. This lets a single proposal mix references to existing and newly created content; Upstream rejects unknown canonical IDs or patch-local IDs outside proposals.
- Proposal payloads MUST remain within the shared extension set (extension capability intersection negotiated during preamble). Upstream SHOULD reject proposals that include unknown extension content.
- Proposals are serialized by Upstream. On acceptance, Upstream sends `ProposedTransactionReply` (code 0) with remap tables from patch-local → canonical entities/assets, followed by a canonical `Transaction` broadcast that includes the applied changes and any cascade effects. Non-zero codes are rejections (1 denied, 2 resource exhausted, 3 malformed/disallowed, >=256 application-specific). No error message payload is returned.
- Proposal arrival order is preserved per Downstream connection. Upstream chooses a total serialized order across connections and may interleave their streams. On the proposer connection, a code-0 reply is immediately followed by its canonical transaction with no unrelated transaction between them; other Downstreams receive only the canonical transaction.
- Concurrent proposals use last-accepted-proposal-wins semantics for complete component replacement. Downstreams are encouraged to use separate entities for independently authored observations, annotations, results, and other contributions rather than sharing mutable component lists or maps.
- A rejected proposal has empty remap tables and no corresponding transaction. Malformed proposals normally receive code 3 without closing the connection; repeated malicious or resource-exhausting input may cause disconnection.
- Proposal transaction IDs must be unique among that Downstream's in-flight proposals and may be reused after a reply. On acceptance, Upstream replaces patch-local IDs everywhere they occur in recognized payloads; every created placeholder is mapped, and any referenced but uncreated placeholder causes rejection.
- Patch-local entity IDs are unique within a proposal; patch-local asset IDs are scoped by asset type. Remap sources have bit 63 set, destinations have it clear, destinations are newly allocated and unique, and asset mappings preserve `AssetTypeID`.
- Remap tables support at most 65,535 entity creations and 65,535 asset creations per proposal. Larger proposals are rejected with resource-exhaustion code 2 and should be split into smaller updates.
- Upstream deeply parses shared-extension proposal payloads to replace embedded IDs. It may normalize accepted representations, but materially different operations are rejected. Authority-generated cascade changes appear in the canonical transaction without extra remap entries.
- Downstream may display pending proposal-local state optimistically. Rejection removes or reverts it; acceptance remaps it and the immediately following canonical transaction replaces it. Only canonical content is authoritative.
- Asset payloads in proposals must be inline buffers (no URLs or OOB) from Downstream to Upstream. Asset discard and deletion proposals are rejected.

## RPC and Interaction

- Entities can advertise callable endpoints with the `RPCEndpointComponent`. Argument, terminal-result, and optional stream-result hints include descriptions, semantic tags, and informational `Any` types; applications may validate them, but the protocol layer does not enforce them. Arguments and results are carried as `Any` values. `Any` maps use unique string-or-integer keys; receivers may reject, ignore, or overwrite duplicate keys and may represent maps as ordered key/value pairs.
- `RPCInvoke` travels Downstream to Upstream and references the target entity. `RPCReply` travels Upstream to Downstream and returns zero or more `RPCStreamResult` values followed by exactly one terminal `RPCComplete` or `RPCError`. Replies for unknown or completed invocation IDs are protocol violations.
- Deleting an endpoint entity does not cancel an invocation already accepted by Upstream. The invocation may complete normally or return an error; new invocations against the deleted endpoint are errors.
- `RPCCancel` is a best-effort request for a nonzero in-flight invocation. Cancellation races are harmless: unknown or completed IDs are ignored, the ID remains in flight, and Upstream still sends exactly one terminal reply unless the transport closes.
- RPC side effects on authoritative ECS state are communicated through ordinary `Transaction` messages rather than RPC replies.
- `InteractiveComponent` gates `CanTranslate`, `CanRotate`, `CanScale`, `Activate`, and optional conservative entity-local interaction bounds. Bounds may be an AABB or sphere. Without `InteractionBoundsComponent`, clients must not spatially pick the entity merely because it is interactive, though non-spatial UI may still expose its actions. Transform limits apply in parent-local space; rotation is unrestricted or constrained to one axis relative to a reference orientation. Buttons and entity activation use a shared action type that either invokes an RPC or emits a registered Event with arguments. The first activation action is the default; remaining entries are alternatives.
- `IsSelectedComponent` is shared authoritative selection state; private selection may remain client-local, and multiple entities may be selected.
- Parameter UI uses an acyclic entity graph with separate matching definition and state components. Disabled state inherits to descendants. Buttons execute RPC or Event actions without changing their empty state. Upstream validates downstream value and structural proposals; malformed date/time proposals may be rejected, while clients may ignore or display malformed authoritative values.

## Semantics, Observations, and Tasks

- Entities may carry human-readable descriptions, compact whitespace-delimited semantic identifiers, and semantic `Any` properties. Leading, trailing, and repeated ASCII whitespace in tag lists is ignored; an empty list has no tags, and tag identity is exact token bytes without URI or Unicode normalization. Semantic property keys are stable string identifiers; integer map keys are not valid for this component.
- Relationship meanings are entities marked with `RelationshipDefinitionComponent` and described through ordinary name, description, tag, property, and relationship components. Each relationship points weakly to both its definition and target. Weak references may be absent or deleted, are resolved only while alive, and never retain an entity or block deletion. Semantic metadata and relationships do not inherit through `ChildOfComponent`.
- `ParticipantComponent` marks a known actor without identifying a transport connection or asserting that it is currently connected. A Downstream may propose zero or many participant entities—for example, one immersive client may represent several tracked people—and disclose selected names, descriptions, semantics, transforms, or other components. Connection establishment does not implicitly create a participant. Participant transform updates should remain low rate, with approximately 4 Hz or less recommended and local interpolation used for smoother presentation. Shared participant metadata and relationships such as `CreatedBy` are semantic assertions, not authenticated identity or provenance.
- Observations are marker entities. They may identify a weak subject and a conservative entity-local AABB or bounding sphere, while semantic tags and properties carry measurement, inference, unit, time, uncertainty, confidence, and provenance information. A zero-volume box or zero-radius sphere denotes a point observation.
- Tasks retain a state component from pending through a terminal succeeded, failed, or cancelled state; terminal states cannot transition again. Results are separate entities carrying `ResultComponent` and a relationship whose definition has the well-known `noodles.relationship.result_of` tag and whose target is the task. This permits zero, one, or many results. `Any::Void` explicitly records a result with no returned value, while absence of `ResultComponent` records no result on that entity.
- Well-known semantic contracts are maintained in [the semantic registry](spec/well_known_semantics.md). Adding a new identifier is compatible; changing an existing identifier's meaning is not.

## Events

- Events are transient broadcasts with `Any` arguments. They are not stored in snapshots or replayed to late joiners; durable facts belong in ECS components or assets.
- `EventRegistryComponent` declares event identifiers, unique names, descriptions, semantic tags, and informational argument hints in a namespace scoped by its entity. The wire address is `(registry entity, definition_identifier)`, where `definition_identifier` selects an explicitly identified definition rather than indexing the registry sequence, so entries may be reordered or removed without changing the identity of the remaining events.
- Any Upstream or Downstream may emit a registered event, subject to Upstream policy. A Downstream sends it to Upstream without dispatching locally; Upstream validates and orders it with transactions and other events, handles it locally once, and broadcasts it unchanged to every Downstream including the sender. The sender dispatches the returned broadcast normally.
- Per-Downstream arrival order is preserved and Upstream chooses interleaving across connections. Unknown registry/type IDs are invalid. Upstream may drop, rate-limit, or disconnect under application/resource policy.
- Events are scoped to one Upstream session. Automatic relay across separate Upstream sessions is not defined.
- Protocol-wide semantic event contracts are registered in the [well-known events document](spec/well_known_events.md). Their versioned names are stable, while numeric definition identifiers remain local to each registry. The initial `noodles.view.pose.v1` contract provides optional world-space view-pose telemetry and is limited to 4 Hz per registry entity.

## Assets and OOB Transfer

- Assets are addressed by `AssetTypeID` + `AssetID`. Large, immutable payloads can flow over optional out-of-band (OOB) asset channels (`OOBAssetInfo` in the preamble) to avoid blocking the main communication socket.
- `(AssetTypeID, AssetID)` is a stable logical identity. `ContentModifyAsset` creates or atomically replaces its value and places a strong reference in the active asset table. Existing references observe replacement without being rewritten. Clients invalidate decoded, cached, and GPU representations of the previous value, but may retain them internally until outstanding work finishes.
- A buffer replacement must leave all active `BufferView` references in bounds when its transaction commits. Inline replacements are ordered by the transaction. Each changed OOB-backed revision uses a fresh `OOBAssetID`; each changed URL-backed revision uses a distinct immutable URL.
- `ContentDiscardAsset` drops the active asset table's strong reference but retains weak lookup. Component and asset references remain strong. A later reference may use the discarded asset only while another live protocol reference keeps weak lookup resolvable; otherwise Upstream republishes it with `ContentModifyAsset` first.
- `ContentDeleteAsset` follows discard and removal of the final protocol-visible reference. It removes weak lookup and tombstones the identity. Private renderer/decoder references may finish asynchronously but are no longer protocol-addressable. A late joiner's snapshot publishes assets still held by world references and then reproduces their discarded state before `InitialStateComplete`.
- `BufferView` references a `BufferAssetID`; higher-level assets (mesh, image, environment, material) reference buffer-backed views.
- Image formats are detected from encoded-byte signatures. Core clients must support PNG and JPEG; WebP, KTX2, Radiance HDR, and OpenEXR are optional. Unsupported optional codecs and environment-rendering features may be ignored without treating the connection as malformed.
- Environment image indices must be in bounds when enabled. Material image tables contain at most 16 entries, and every enabled texture index must address an existing entry; disabled reference fields are ignored.
- PBR color and emission maps are sRGB. Roughness/metallic, occlusion, normal, and other non-color maps are linear. Clients may ignore or approximate unsupported extended material features without rejecting the scene.
- Immutable `PBRMaterialFeatureSet` assets declare shader/pipeline topology for early compilation and caching. `PBRMaterial` assets are parameter instances sharing a feature set. Base-only instances omit the extended parameter block; its presence must agree with the referenced feature set.
- OOB support is provided through one or more WebSocket channels, where asynchronous asset requests can be made. These may be kept open as long as the main channel is open. Content uses the JAW binary format, but with the restricted `OOBRequest` and `OOBReply` message sets.
    - `IntroductionReply` supplies an unpredictable 256-bit bearer token for the main-channel session. Every OOB request must contain that token, and it expires when the main channel closes.
    - OOB asset IDs are scoped to the main-channel session and remain valid while referenced by a live buffer asset. They are never reassigned to different bytes during that session, although multiple buffer assets may intentionally share the same OOB payload.
    - Connections must not be maintained with a node that is not connected on the main channel.
    - A request is made by a `OOBRequest`, and serviced by a single `OOBReply` response.
    - An asset occupies one logical reply. WebSocket fragmentation and incremental consumption are allowed, but application-level chunk numbering and resumable transfer are not defined.
    - A buffer asset may contain an OOBAssetID, or blob of bytes, that can be requested from Upstream.
    - OOB errors include the requested `asset_id` plus a UTF-8 reason string.
    - Receivers may reject URL schemes, origins, redirects, credentials, or other fetch behavior according to local security policy.

## Spatial Audio

- Optional Audio extension `0x0001` depends on Core and provides buffer-backed `AudioAsset` clips plus `AudioSourceComponent` spatial point sources. Peers without audio support do not advertise it.
- Baseline audio is mono or stereo signed PCM16 RIFF/WAVE. Spatial evaluation uses a mono signal; stereo baseline content is downmixed before playback. Other codecs and channel layouts are optional.
- Playback is anchored to the authority time estimate with a start time, clip offset, positive playback rate, playing state, and looping state. This permits synchronized playback and lets late joiners evaluate the current playhead.
- A source is located at its entity's world-space origin. It is omnidirectional, unaffected by its own rotation, scale, or visibility, and attenuates linearly from full gain at `min_distance` to silence at `max_distance`. The listener is local downstream view state rather than authoritative ECS state.
- Directional cones, Doppler, filters, reverb, and protocol listener components are not defined. Clients may mute, defer, limit, or reject audio for resource, safety, accessibility, autoplay, or platform policy.

## Animation and Deformation

- Core includes an `AnimationAsset` (samplers/channels) plus `AnimatorComponent` for layered playback.
- Keyframe times are finite and strictly increasing; sampler indices and buffer sizes are exact. Compact `SRGBA8` colors are interpolated in linear space and do not support cubic interpolation.
- Asset animation affects every entity referencing that shared asset. Per-entity material animation requires a distinct material asset identity. Producers should avoid multiple animators targeting the same path; clients encountering that conflict may choose one.
- Transform animation and transform sampling on the same entity are strongly discouraged. Clients may choose how to resolve the conflict; ignoring sampling and evaluating the animation is recommended.
- Mesh assets now describe joint/weight attributes for skinning and optional morph targets; `SkinComponent` supplies joint palettes and inverse bind matrices.
- `MorphWeightsComponent` carries per-entity weights; animation channels may target specific weights by index.
- Animation time estimates remain monotonic within a session. Entity names are display-only and need not be unique. Text uses local-unit height and sRGBA color; clients may ignore authored alpha and render it as opaque.

## Extensions

The specification is designed for expandability and modularity through Extensions. Extensions define a collection of components, assets, and semantics with optional dependencies on other Extensions.

- During handshake, each side advertises supported extensions. Their intersection is the shared extension set for the link.
- Capability-list order and duplicate entries have no meaning. Every peer must advertise Core (`0x0000`) and all dependencies of every extension it advertises.
- Normative directionality:
    - Downstream -> Upstream proposals MUST only contain types in the shared extension set.
    - Upstream -> Downstream state updates SHOULD prefer the shared extension set for interoperability; unknown extension payloads MAY be skipped/ignored by receivers.
- Receivers skip unsupported component batches and asset payloads as complete length-delimited units; associated deletions and discards are ignored and no placeholder local state is created. Reference validation applies only to understood state.
- An unknown type identifier within a negotiated extension is a protocol violation. Old and new extension IDs may coexist, but should not encode the same logical component on one entity unless their specifications define that behavior.
- Extensions are registered as 16-bit namespace IDs for components and assets. The registry is tracked in `spec/registry.md`.
- The Core extension (0x0000) defines the base component and asset types in `spec/core`. All implementations must support this core extension.

## Message Direction and Failure

- `Transaction`, `InitialStateComplete`, `ProposedTransactionReply`, and `RPCReply` travel Upstream to Downstream. Replies are sent only to the relevant proposer or invoker.
- `ProposedChangeTransaction`, `RPCInvoke`, and `RPCCancel` travel Downstream to Upstream. `EventMessage` travels in both directions according to its broadcast flow. Any other message received in the wrong direction is a protocol violation and closes the connection.
- Malformed top-level or authoritative content closes the connection. Invalid proposals may instead be rejected with code 3. Policy denials use code 1 and are not protocol violations.
- No protocol goodbye is required; transport closure may occur at any time.


## Status and Roadmap

- Development-only: protocol version 0 explicitly forbids production use, and its Base/Core wire layouts may change without incrementing `protocol_version`. The first supported release increments it to 1. Afterward, every incompatible Base/Core change increments `protocol_version`, while incompatible optional-extension changes receive new extension IDs.
- The compatible JAW compiler commit is recorded in `JAW_REVISION`. Run `./scripts/check-spec.sh` to generate and validate the complete specification; set `JAW_BIN` when the compiler is not installed or built in the adjacent `jaw` repository. GitHub Actions runs the same check against the pinned compiler. Further cross-language fixture plans are recorded in `CI_NOTES.md`, and [implementation guidance](IMPLEMENTATION_NOTES.md) describes a suggested runtime architecture.
- Systems (runtime behavior), richer capability negotiation, and transport options (e.g., libp2p) remain on the roadmap.
- Feedback and implementation notes are welcome; keep the `.jaw` schemas and markdown docs in sync as behavior evolves.
