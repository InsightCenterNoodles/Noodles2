# Implementation Notes

This document is non-normative. JAW schemas and normative specification prose
take precedence.

## Suggested layers

1. **Transport adapters** provide bounded main/OOB records and raw OOB streams.
2. **Wire codec** decodes generated JAW values under local resource limits.
3. **Membership/router** maintains parent, child-subtree NodeID routes, pending
   join UUID routes, and connected/disconnected membership.
4. **Replication adapter** observes local owner-written ECS changes, emits
   `ContentBatch`, and applies received/snapshot content directly to the engine.
5. **Presentation/application systems** resolve references, proposals, events,
   RPC, animation, audio, and rendering policy.

Sessions are implicitly trusted in version 0. Do not mistake origin NodeIDs for
cryptographic authentication.

## Ownership and engine mapping

Derive allocator as the high 32 bits of every entity/asset ID. Current owner is
that allocator while connected and the Bookkeeper after `PeersDisconnected`.
Maintain the usual protocol-ID to engine-handle map; engine generation/index
handles need not equal wire IDs. Only emit modifications/deletions for IDs
currently owned by the local NodeID. Creation additionally requires the local
NodeID to be the encoded allocator: the Bookkeeper never allocates fresh IDs in
an inherited retired range. Never recycle a wire ID or typed asset identity
during one session even when the engine recycles its private handle.

Apply local changes normally, then use engine change detection to batch complete
component replacements, additions/removals, creations/deletions, and asset
changes. A batch is serialization grouping, not an atomic overlay. Received
batches can be applied directly; no optimistic/pending scene is required.

References may outlive or precede their targets. Store protocol IDs where useful
and resolve through the live handle map at use time. Missing references should
degrade the affected feature rather than creating placeholder entities.

## Broadcast and routing

Broadcast scene batches and events over every edge except their arrival edge.
Because the topology is a strict tree, this reaches every connected peer once
without loop IDs. Preserve transport order while forwarding messages from one
origin.

For routed RPC/OOB traffic, consume self destinations, choose a child route when
the destination belongs to that subtree, and send parentward only for a known
destination elsewhere. Never reflect traffic over its ingress edge. Install
routes while a successful `PeerJoined` returns along its pending JoinRequest UUID
path. Remove routes behind a disconnected child edge and resolve those retired
allocator NodeIDs to the Bookkeeper. Boundedly hold an unknown destination only
when a membership update may be racing; otherwise fail it.

## Snapshot capture

Take snapshots at a stable replication boundary. Stop local ECS mutation and
stop consuming other main links, send `SnapshotBegin`, all `SnapshotBatch`
records, and `SnapshotComplete` serially, then resume. Network transports provide
the temporary buffering; no protocol catch-up queue exists. OOB service may
continue independently. Closing an over-limit join is safer than blocking the
session indefinitely.

Many engines need no explicit pause machinery. With a `First` / `Update` / `Last`
schedule, process incoming protocol work in `First`, local systems in `Update`,
and emit change detection in `Last`. A snapshot handled serially in `First` sees
a stable ECS whose preceding local changes were already emitted in the previous
`Last`. Other adapters may flush or pause as required, but the wire semantics are
the same. `SnapshotComplete` is the sole publish-ready boundary.

Snapshots contain only live ECS plus connected and historical membership needed
to resolve allocator ownership. No owner component is needed. Deleted IDs need
not be transferred because IDs are never reused. Omit inactive inherited
proposal entities.

## Proposal systems

Treat entities carrying `ProposalTargetComponent` as declarative inputs. Query
typed proposal components whose target owner is local. Application systems may
combine them however appropriate, but only resulting writes to locally owned
target entities replicate as real scene changes. Do not generate protocol
accept/reject state.

ProposalTarget is immutable. A proposer that wants another target deletes the
old proposal entity and creates a new one. On `PeersDisconnected`, the
Bookkeeper inherits affected live identities. Inherited proposal entities are
inactive and should be deleted; ordinary scene state should normally remain and
may be modified or deleted under Bookkeeper policy.

## OOB streaming and cache

Establish and authenticate the parent OOB link before forwarding `JoinRequest`.
If it drops while the main link remains open, reconnect with the same token; the
new authenticated connection replaces the old one. Boundedly queue or fail
fetches during the interruption. Continued failure may close the main edge under
local policy, but a brief OOB interruption does not stop main ECS traffic. Avoid
publishing new OOB-backed buffers until the required route is restored.

Validate `byte_size` before allocation and stream through fixed-size buffers.
Hash while forwarding or consuming, and publish/cache bytes only after the final
SHA-256 matches. A cache hit may answer on the request path without contacting
the owner. A relay can forward without caching.

Keep OOB control and its following raw payload adjacent in one direction. Apply
backpressure per link; do not let a slow child create unbounded buffers. Cache
eviction is private and never changes replicated asset state.

## Session time

On every SessionTime replacement, record `(session_time, local_monotonic_now)`.
Extrapolate at rate 1 and smooth offset corrections without moving exposed time
backward. Treat it as approximate presentation time, never as causal ordering.

## Resource and failure policy

Bound message sizes, queued joins, snapshot bytes, RPCs, events, decoded assets,
OOB streams, pending routes, and proposal entities. A peer may
ignore unsafe content or close a link. Closing a parent link ends the local
subtree's session; closing a child removes that entire subtree from the surviving
session after Bookkeeper membership propagation.

Generated code compiling is only the first check. Test malformed framing,
truncation, wrong allocators/current owners, invalid component encodings,
routing loss, stable snapshot boundaries, OOB reconnection/digest mismatches,
Bookkeeper inheritance, and disconnected-subtree cleanup.
