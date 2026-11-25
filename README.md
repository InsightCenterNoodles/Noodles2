# NOODLES2

**NLR Object Oriented Data Layout and Exploration System 2**

NOODLES2 is a compact binary protocol for synchronizing an ECS-based visual
scene among trusted peers. Connections form a strict rooted tree. Each live
entity and typed asset has one current writer, while explicit proposal entities
let other peers publish typed intent for that owner to consider. When a peer
disconnects, the Bookkeeper inherits its live identities.

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHOULD**, **SHOULD NOT**,
**MAY**, and **OPTIONAL** are interpreted as described by RFC 2119 and RFC 8174
when they appear in all capitals.

## Protocol model

- Every connected node is a peer. The first node is the Bookkeeper and has
  `NodeID` 0.
- The Bookkeeper controls membership, allocates never-reused NodeIDs, fixes the
  session extension set, and publishes coarse session time. It does not order or
  approve ordinary scene changes.
- Main links form a strict rooted tree. A node has one parent and zero or more
  children. Losing the path to the Bookkeeper ends that subtree's session.
- Sessions are implicitly trusted in protocol version 0. Node identity and
  ownership rules provide correctness boundaries, not cryptographic security.
- ECS changes, events, and membership announcements broadcast through the tree.
  RPC and OOB requests use NodeID routes.

## Wire format and framing

- JAW schemas define little-endian binary layouts. JAW `bool` occupies one byte
  and only `0` and `1` are valid.
- One top-level JAW value occupies one logical message on message-oriented
  transports. Byte-oriented transports prefix every JAW record with its `u32`
  little-endian payload length.
- A zero record length, truncated record, parse failure, trailing bytes, or
  length over local limits is fatal to that link. Byte streams do not attempt
  resynchronization.
- OOB success controls are followed by one non-JAW raw payload. Byte-oriented
  links prefix it with a `u64` length; message-oriented links use the following
  logical message boundary.
- Nodes MAY impose implementation resource limits and disconnect when content
  is malformed, unsafe, or exceeds them.

## Membership and connection flow

1. A child opens a main link to an existing peer and sends `Introduction` with
   its protocol version and supported extensions.
2. The parent returns its NodeID, the Bookkeeper-fixed session extension set,
   and endpoints/token for the bidirectional OOB link.
3. The child establishes the OOB link and sends `OOBLinkIntroduction` as its
   first control record.
4. The child generates a fresh UUIDv7 and sends `JoinRequest` as its ready
   message. The parent forwards it toward NodeID 0 only after authenticating and
   associating the OOB link; an early request waits locally.
5. Bookkeeper verifies complete session-extension support, assigns a nonzero
   NodeID, and broadcasts `PeerJoined`.
6. Arrival of that record at the joining UUID is its successful response. Its
   parent supplies a complete ECS snapshot.
7. The child may publish immediately after receiving `SnapshotComplete`.

NodeIDs and UUIDs are never reused in one session. Main-tree reconnection is not
defined: a returning implementation generates a new UUIDv7 and receives a new
NodeID. Replacing an OOB transport while the main link remains open is not a
session reconnection.

OOB support is mandatory. A temporary OOB interruption does not close the main
link: the child reconnects with the same token, and the newest authenticated OOB
connection replaces the broken one. Fetches may be boundedly queued or failed
during the interruption. Either peer may close the main edge if OOB cannot be
restored within local policy. Parents advertise their normal endpoint set rather
than customizing scene streams per child.

The Bookkeeper fixes one immutable session extension set. Every joining peer
MUST implement all of it; peers may support additional extensions locally but
MUST NOT publish their types into the session. This ensures every peer can
serialize a complete snapshot for a future child.

### Subtree loss

When a child edge closes, the surviving parent reports that child and its known
descendants to the Bookkeeper. Bookkeeper broadcasts `PeersDisconnected` to the
surviving tree. The separated subtree ends its session immediately. Assigned
NodeIDs remain reserved. `PeersDisconnected` transfers the departed allocators'
live entities and typed assets to the Bookkeeper without changing their IDs.
The Bookkeeper may retain, modify, or delete inherited content according to
policy; retaining ordinary scene state is recommended. Inherited proposal
entities are inactive and should be deleted. Retained replicated values remain
unchanged until the Bookkeeper edits them, while presentation systems such as
animation, audio, and bounded transform sampling continue evaluating normally.
RPC endpoint components should be removed unless the Bookkeeper implements them.

If Bookkeeper disconnects, the entire session ends.

## IDs and ownership

`EntityID` and `AssetID` divide their `u64` value as:

```text
63                    32 31                     0
+-----------------------+-----------------------+
| allocator NodeID (u32)| allocator-local (u32) |
+-----------------------+-----------------------+
```

Asset IDs remain scoped by `AssetTypeID`, so each node has `2^32` IDs per asset
type. Only the encoded allocator may create identities in its range. While it is
connected it is their owner; after `PeersDisconnected`, NodeID 0 owns its live
identities. The Bookkeeper may modify or delete them but cannot allocate new IDs
in the retired range. IDs are never reused during the session.

Current ownership is derived from the encoded allocator plus the membership
roster; no Owner component exists. Snapshots retain disconnected PeerRecords so
late joiners can distinguish retired allocator ranges from NodeIDs that never
existed.

## ECS replication

A node changes its own engine ECS first and emits `ContentBatch` from ordinary
change detection. Batches save framing/serialization overhead but are not
transactions:

- Operations apply in array order and may be observed incrementally.
- Creation requires `origin` to be the encoded allocator. Other operations
  require `origin` to be the target's current owner.
- Every receiver applies it once and forwards it over every tree edge except the
  arrival edge.
- Reliable ordered links and the strict tree preserve each origin's order.
- Changes from different owners affect disjoint owned state and require no
  Lamport clock, global counter, acknowledgement, rollback, or pending ECS.

Component modification is complete replacement. Entity creation is explicit;
component modification never implicitly creates an entity. Create then delete
in one batch is legal and permanently consumes the ID. Deleting unknown/already
deleted state is a no-op.

References are stored as IDs and may name missing or dead entities/assets.
Consumers MUST check liveness before use. Deletion does not cascade into content
owned by other peers. Engine tombstones may be retained locally, but deleted IDs
need not be serialized in snapshots because IDs are never reused.

Self-parenting is illegal and the complete live `ChildOf` graph MUST be acyclic,
including across allocators. Proactive full-graph cycle scans are optional, but
every traversal must be cycle-safe or resource-bounded. A peer detecting a cycle
may terminate its participation in the session. A missing/dead parent makes its
edge inactive. Reparenting has keep-local semantics: transform and sample values
are reinterpreted in the new effective parent frame without conversion.

Parameter child graphs are also acyclic, but every child reference must remain
within the containing entity's allocator range. This makes their structural
ordering single-writer. Structural cycle traversal follows the same safety rule;
general semantic `Relationship` graphs are not required to be acyclic.

## Snapshots

The joining peer's direct parent supplies a snapshot at a stable replication
boundary. It stops local ECS mutation and stops consuming other main links while
serially sending:

1. `SnapshotBegin` with the session membership roster.
2. Zero or more `SnapshotBatch` values containing current live entities,
   components, typed assets, active proposal entities, and the Bookkeeper session
   entity.
3. `SnapshotComplete`, the single publish-ready boundary.

Snapshot batches use creation and modification operations only. Every live
entity is created once before any component publication; deletions and tombstones
are not included.

The parent then resumes normal engine and main-link processing; transport-buffered
membership, content, events, and RPC flow afterward as ordinary live traffic.
Snapshot messages are link-local and are not relayed. An engine may satisfy the
stable boundary naturally—for example, process incoming protocol work in
`First`, local systems in `Update`, and change publication in `Last`. Handling a
snapshot serially in `First` needs no extra flags, flushing, or background queue.
Other engine adapters may briefly pause or flush as needed. If snapshot capture
exceeds safe resource limits, the joining connection may be closed and must join
again with a new identity.

## Proposal entities

Only owners write real target state. Other peers express typed intent through
ordinary proposer-owned ECS entities carrying immutable `ProposalTarget` plus
one or more proposal components:

- `TransformProposal`
- `ParameterValueProposal`
- `SelectionProposal`

Each proposer uses a separate entity, so concurrent proposals do not overwrite
one another. Proposals are active declarative input, not pending transactions:
the proposer may update or withdraw them, while the target owner may ignore,
choose, merge, average, clamp, or reinterpret them. No acceptance/rejection
reply exists. If the owner acts, it changes the real component in its own ECS and
that ordinary owner-authored change replicates normally. A TransformProposal is
interpreted in the target's effective parent frame when considered; owners should
ignore transform proposals while reparenting a target.

Proposal entities inherited by the Bookkeeper are inactive and should normally
be deleted. Proposal components grant no authority.

## Events

Events are transient broadcasts and are not snapshotted or replayed. The source
dispatches locally once and sends to all tree neighbors. Each receiver dispatches
once and forwards over every edge except the arrival edge. Per-source order is
preserved; there is no global event order. Registry definitions and argument
hints remain ordinary owner-written ECS state.

## Routed RPC

RPC messages carry source and destination NodeIDs. The destination is the
endpoint's current owner. A route is selected as:

1. Consume locally if destination is self.
2. Send to the child link whose known subtree contains destination.
3. Send parentward only when the destination is known elsewhere.

Join traffic establishes descendant-to-child routes; disconnection removes
them, and a disconnected allocator resolves to NodeID 0 as current owner. A
message is never reflected onto its ingress edge. Unknown destinations are
boundedly held for racing membership or failed. Only the destination executes or
replies. Invocation identity is `(source, invocation_id)`. Replies, stream
results, errors, and cancellation use the reverse destination. RPC side effects
are ordinary current-owner-authored ECS changes.

The Bookkeeper returns `RPC_ERROR_TARGET_UNAVAILABLE` for a late request it
cannot execute; the caller may resolve ownership and retry. An invocation that
was already in flight when its target disconnected terminates locally. Its
outcome may be unknown because the former owner could have executed it before
loss, so non-idempotent calls must not be retried automatically.

## Assets and routed OOB transfer

Buffer contents may be inline, fetched from immutable URLs, or represented by
`OOBContent { byte_size, sha256 }`. The containing BufferAsset's current owner is
the fetch destination.

OOB requests follow the same subtree routes as RPC. Every peer MUST forward OOB
controls and stream payloads without requiring the complete payload in memory.
Caching is OPTIONAL:

- A relay with verified matching bytes may answer and stop forwarding.
- Success contains the responder and original requester destination, followed by
  one raw payload.
- Requesters verify exact length and SHA-256 before accepting bytes.
- Cache entries are naturally shared across owners for identical content.
- After allocator disconnection, requests route toward the Bookkeeper; if it and
  on-path caches lack the bytes, retrieval fails.

OOB traffic is never reflected onto its ingress edge. A request racing a main
membership update may be held in a bounded pending-route queue and then failed.
During a temporary OOB interruption, affected fetches may similarly queue or
fail and be retried once the replacement OOB link authenticates. Main ECS
traffic continues, but a peer should avoid publishing new OOB-backed buffers
while its required route is unavailable.

OOB control/payload adjacency, framing, transport fragmentation, and truncation
rules are specified in `spec/oob_asset.jaw`.

## Session time

The Bookkeeper owns well-known EntityID 0 and replaces its `SessionTimeComponent`
approximately once per second with finite monotonic seconds since session start.
A receiver anchors that value to its local monotonic receipt time and extrapolates
between updates. Corrections are smoothed and exposed session time never moves
backward.

Session time is approximate because it includes relay latency. It supports
ordinary animation, transform sampling, audio, and timestamped events, but is
not used for conflict resolution or message ordering. Precision synchronization
may be added later as a routed time-probe extension.

## Core scene semantics

- Coordinates are right-handed and mesh front faces are counterclockwise.
- An absent transform is identity. Ancestor invisibility suppresses descendants.
- `BufferView` and other asset references are usable only while targets resolve.
- Core images require PNG and JPEG decoding; other documented codecs are
  optional.
- `PBRMaterialFeatureSet` assets are immutable; a topology change uses a new ID.
- Parameter definition/state tags match and owner policy decides how typed
  parameter proposals affect state.
- Animation targets are evaluated locally against Bookkeeper session time.
- Unsupported presentation features may be ignored or approximated when the
  underlying extension is nevertheless implemented.

## Extension registry and validation

Core is extension `0x0000`; Audio is optional extension `0x0001` and depends on
Core. Extension and component/asset identifiers are permanent once published;
incompatible optional revisions receive new extension IDs, while base/Core
changes use `protocol_version`.

Run the complete schema check with the compiler revision recorded in
`JAW_REVISION`:

```text
JAW_BIN=<path-to-jaw> JAW_REPO=<jaw-checkout> ./scripts/check-spec.sh
```

The check generates Rust, C++, Python, and Swift bindings and validates each
schema root. Additional runtime guidance is in `IMPLEMENTATION_NOTES.md`.
