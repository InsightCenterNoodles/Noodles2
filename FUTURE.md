# Future and Deferred Design Work

This file tracks deliberately unresolved or revisitable protocol work. Items
here are not part of the current normative specification.

## Proposal application adjacency

An accepted `ProposedTransactionReply` is currently required to be immediately
followed, on the proposer connection, by its canonical `Transaction`. Revisit
whether explicit correlation on the canonical transaction would permit safe
interleaving without making optimistic reconciliation ambiguous.

## RPC lifecycle

- Consider a best-effort `RPCCancel` message for long-running streams.
- Consider standardized Core `RPCError` codes; codes are currently application-defined.

## JAW named variant alternatives

Migrate variants to explicit case names if/when the JAW compiler supports them.
The current compiler at revision `18520eb` derives generated case names from the
payload type and numeric tag.

## Richer rotation constraints

Core currently supports unrestricted rotation or one axis-angle constraint.
Possible future variants include ordered Euler, rotation-vector, and swing-twist
constraints.

## Transfer and transport

- Consider resumable, application-level chunking for very large OOB payloads.
- Consider more detailed codec/render-feature negotiation than extension IDs.

## Rendering and synchronization extensions

- Round-trip clock synchronization for applications needing tighter timing.
- Portable font assets and deterministic text layout.
- Defined spatial blending semantics for non-global environments.
- Additional 2D-specific rendering and interaction facilities.
