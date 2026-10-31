# Future and Deferred Design Work

This file tracks deliberately unresolved or revisitable protocol work. Items
here are not part of the current normative specification.

## Proposal application adjacency

An accepted `ProposedTransactionReply` is currently required to be immediately
followed, on the proposer connection, by its canonical `Transaction`. Revisit
whether explicit correlation on the canonical transaction would permit safe
interleaving without making optimistic reconciliation ambiguous.

## RPC lifecycle

- Consider standardized Core `RPCError` codes; codes are currently application-defined.

## JAW named variant alternatives

Migrate variants to explicit case names if/when the JAW compiler supports them.
The compiler revision pinned by `JAW_REVISION` currently derives generated case
names from the payload type and numeric tag.

## Richer rotation constraints

Core currently supports unrestricted rotation or one axis-angle constraint.
Possible future variants include ordered Euler, rotation-vector, and swing-twist
constraints.

## Transfer and transport

- Consider resumable, application-level chunking for very large OOB payloads.
- Consider more detailed codec/render-feature negotiation than extension IDs.

### Local asset sources

Consider an optional local-asset-source extension for deployments where large
buffer contents are already accessible to the Downstream. This is distinct from
ordinary OOB transfer: OOB channels deliver requested bytes, while a local asset
source imports bytes from storage or memory already available in the receiver's
environment. Filesystem and shared-memory sources should therefore not be added
as `OOBEndpoint` transport variants.

Two mechanisms warrant separate designs:

- **Shared storage:** identify a configured logical storage namespace plus a
  relative path, byte offset, byte length, and immutable revision or content
  digest. Each participant maps the logical namespace to its own mount point;
  the Upstream must not assume that its absolute filesystem path is meaningful
  to a Downstream. The design must prevent in-place mutation from changing the
  bytes of an already published asset revision.
- **Transferred local handles:** allow a local IPC binding to transfer a
  read-only file or shared-memory handle, such as an open descriptor passed over
  a Unix stream socket with `SCM_RIGHTS`. Define platform bindings, mapping
  offset and length, ownership, sealing or immutability, permissions, lifetime,
  cleanup, and failure behavior. Prefer transferred handles over globally named
  shared-memory objects where practical.

The extension must negotiate resolver support explicitly. It should also define
how a producer supplies a portable fallback, such as normal OOB or HTTP, when
not every recipient can access the local source. This may require ordered
alternative sources for one logical `BufferAsset`; the current single-source
representation does not provide fallback. Private `file:` or application URL
schemes may be useful for experiments but are not a portable shared-storage
contract by themselves.

## Rendering and synchronization extensions

- Round-trip clock synchronization for applications needing tighter timing.
- Revisit producer-supplied transform velocities, acceleration, Hermite curves,
  and standardized extrapolation-correction blending if sample-derived linear
  prediction proves insufficient.
- Portable font assets and deterministic text layout.
- Defined spatial blending semantics for non-global environments.
- Additional 2D-specific rendering and interaction facilities.
