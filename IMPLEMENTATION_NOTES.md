# Implementation Notes

This document is non-normative guidance for implementers. The `.jaw` schemas and
normative specification prose take precedence.

## Suggested layers

Keep four concerns separate:

1. **Wire codec** — generated JAW encoding and decoding, guarded by local limits.
2. **Semantic validator** — IDs, references, numeric constraints, component
   relationships, lifecycle transitions, and direction rules.
3. **World applicator** — stages and atomically commits valid transactions,
   maintains scene generation, reverse references, tombstones, and snapshots.
4. **Authority policy** — accepts or rejects proposals, RPCs, events, assets,
   and resource use according to the application and trust boundary.

A parse failure, invalid authoritative transaction, malformed proposal, and
policy denial have different protocol outcomes; avoid collapsing them into one
generic error path.

## Atomic application and snapshots

Validate and stage a complete authoritative transaction before exposing any of
it to rendering, queries, systems, or callbacks. Reverse-reference indices make
entity deletion, asset lifecycle checks, and replacement validation practical.

For a new Downstream, choose one consistent world generation, serialize that
snapshot, send `InitialStateComplete` with the same generation, then release
queued live transactions and events in order. Do not expose or emit live events
to that Downstream while its registries and referenced state are incomplete.

Complete component replacement is last-accepted-proposal-wins. Prefer separate
entities for independently authored observations, annotations, relationships,
and task results so collaborators do not perform conflicting read-modify-write
updates on one component.

## Asset ownership

Model an asset identity as a stable shared handle whose payload may be replaced:

- The active asset table holds one strong root after `ContentModifyAsset`.
- Understood component and asset references hold strong references.
- `ContentDiscardAsset` drops the table root but leaves weak lookup.
- `ContentDeleteAsset` is legal after discard and after the final
  protocol-visible strong reference has been removed; it removes weak lookup
  and tombstones the ID.

Internal GPU work, decoders, or application caches may retain private references
after protocol deletion. They must not make the deleted identity addressable
again. If a discarded asset is no longer weakly resolvable, Upstream republishes
it with `ContentModifyAsset` before introducing another reference.

## Data and coordinates

The synchronized scene is a renderable `f32` representation, not necessarily
the source scientific dataset. Applications should transform physical or
geospatial coordinates into stable local scene frames. Large source datasets
normally remain in domain stores or query engines and are linked through
semantic metadata and application RPCs; buffers are appropriate when the bytes
are genuinely needed for rendering or shared interaction.

## Defensive limits

Set explicit limits for message bytes, nested `Any` depth, collection lengths,
entity/component counts, decoded asset sizes, concurrent RPCs, pending
proposals, event rates, and snapshot queues. Use checked arithmetic for every
byte-size and offset calculation. Limits are local policy and may differ between
an embedded viewer and a server, but they should fail deterministically rather
than relying on allocation failure.

## Conformance

Generated code compiling is only the first check. Decode and re-encode canonical
cross-language fixtures byte-for-byte, then exercise semantic and negative
fixtures through the validator and atomic world layer. See `CI_NOTES.md` for the
planned fixture set.
