# CI and Wire-Compatibility Work

GitHub Actions runs `scripts/check-spec.sh` for pushes, pull requests, and manual
dispatches. This file also records the remaining wire-compatibility work that is
not yet automated.

## Compiler pin

CI should check out the JAW commit recorded in `JAW_REVISION` and use that exact
compiler for every schema check.

## Schema generation

Run the same entry point locally and in CI:

```text
JAW_BIN=<path-to-pinned-jaw> JAW_REPO=<jaw-checkout> ./scripts/check-spec.sh
```

The script generates the combined Basic + Core + Audio specification for Rust,
C++, Python, and Swift; generates each root separately; compiles the combined
Rust and C++ output; executes the combined Python output; and type-checks the
combined Swift output. `JAW_REPO` is optional locally, but CI should set it so
the script verifies the checkout against `JAW_REVISION`.

## Cross-language fixtures

Check in canonical binary fixtures for at least:

- Introduction and IntroductionReply, including the parent NodeID, fixed session
  extensions, and OOB link token/endpoints.
- OOB authentication before JoinRequest, PeerJoined, JoinRejected, descendant
  routing, OOB reconnection, and disconnected subtree membership changes.
- An empty snapshot and a multi-batch stable-boundary ECS snapshot ending in the
  publish-ready SnapshotComplete marker, followed by ordinary live traffic.
- Allocator-range entity/asset creation, direct component replacement, deletion,
  dead references, Bookkeeper inheritance, and rejection of wrong-current-owner
  publication or Bookkeeper creation in a retired range.
- Transform, parameter-value, and selection proposal entities from two proposers
  targeting one owner.
- Routed RPC streaming followed by completion, error, cancellation,
  TARGET_UNAVAILABLE, OUTCOME_UNKNOWN, and an unreachable destination.
- Event registry creation and a peer-originated tree broadcast; include
  `noodles.view.pose.v1`.
- Button and entity activation actions covering both RPC invocation and Event emission.
- Inline, URL, and SHA-256 OOB buffer descriptions. Include routed current-owner
  fetch, on-path cache response, temporary link replacement, failure, digest
  mismatch, plus zero-length and nonempty payloads under both message- and
  byte-oriented framing.
- Asset publication, replacement, deletion while referenced, unresolved use,
  and private cache retention.
- PCM16 WAVE audio and a synchronized spatial audio source.
- Transform sampling with interpolation, bounded extrapolation, and a hard-transform history reset.
- Representative Core assets and components, including animation and material
  feature-set/material-instance pairs, relationship definition/result entities,
  semantic discovery hints, and AABB/sphere conservative bounds.

Each generated backend should decode every fixture, validate its values, and
re-encode it byte-for-byte. At least one fixture should be authored by each
backend so the test does not rely on a single implementation as the encoder of
record.

Include negative fixtures for malformed discriminants, invalid booleans,
truncated arrays, excessive guarded lengths, invalid semantic combinations, an
  OOB success without its immediately following raw payload, a truncated raw
  payload, digest/length mismatch, an intervening directional OOB control,
  self-parenting, cyclic structural traversal, and cross-allocator parameter
  children.

## Not currently planned

Automated comparison between Core constants and Markdown registry tables is on
hold. It should not be added until the desired registry-maintenance workflow is
clear.
