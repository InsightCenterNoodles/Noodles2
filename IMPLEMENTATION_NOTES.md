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

Derive owner as the high 32 bits of every entity/asset ID. Maintain the usual
protocol-ID to engine-handle map; engine generation/index handles need not equal
wire IDs. Only emit changes for IDs owned by the local NodeID. Never recycle a
wire ID or typed asset identity during one session even when the engine recycles
its private handle.

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
the destination belongs to that subtree, otherwise send parentward. Install
routes while a successful `PeerJoined` returns along its pending JoinRequest UUID
path. Remove all routes behind a disconnected child edge.

## Snapshot capture

At an engine boundary, pause application just long enough to serialize a stable
snapshot into bounded storage. Continue reading transports into a bounded queue.
After capture, resume the engine and normal relay while snapshot bytes transmit;
buffer only post-cut messages for the joining child. Send those after
`SnapshotComplete`. Closing an over-limit join is safer than allowing unbounded
memory or blocking the session indefinitely.

Snapshots contain only live ECS and current membership. Wire IDs encode owner;
no owner component is needed. Deleted IDs need not be transferred because
owners never reuse them. Omit inactive proposal entities from disconnected
owners.

## Proposal systems

Treat entities carrying `ProposalTargetComponent` as declarative inputs. Query
typed proposal components whose target owner is local. Application systems may
combine them however appropriate, but only resulting writes to locally owned
target entities replicate as real scene changes. Do not generate protocol
accept/reject state.

ProposalTarget is immutable. A proposer that wants another target deletes the
old proposal entity and creates a new one. On `PeersDisconnected`, proposal
entities in affected NodeID ranges may be removed from local ECS while ordinary
owned scene entities remain frozen.

## OOB streaming and cache

Attempt the parent OOB link early so inability to connect is detected before the
scene depends on it. A specialized leaf may continue without it only while
treating OOB-backed assets as missing; it cannot accept children because it
cannot forward their fetches.

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

Bound message sizes, queued joins, snapshot bytes, buffered post-cut messages,
RPCs, events, decoded assets, OOB streams, and proposal entities. A peer may
ignore unsafe content or close a link. Closing a parent link ends the local
subtree's session; closing a child removes that entire subtree from the surviving
session after Bookkeeper membership propagation.

Generated code compiling is only the first check. Test malformed framing,
truncation, wrong owners, invalid component encodings, routing loss, snapshot
cut ordering, OOB digest mismatches, and disconnected-subtree cleanup.
