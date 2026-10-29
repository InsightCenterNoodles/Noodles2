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
C++, and Python; generates each root separately; compiles the combined Rust and
C++ output; and executes the combined Python output. `JAW_REPO` is optional
locally, but CI should set it so the script verifies the checkout against
`JAW_REVISION`.

## Cross-language fixtures

Check in canonical binary fixtures for at least:

- Introduction and IntroductionReply, including OOB session information.
- An empty initial snapshot and `InitialStateComplete`.
- Entity creation and component modification.
- An accepted proposal, complete remaps, and its canonical transaction.
- RPC streaming followed by completion and by error.
- Event registry creation and a downstream-originated event broadcast back to all downstreams, including its sender; include `noodles.view.pose.v1`.
- Inline, URL, and OOB buffer descriptions.
- PCM16 WAVE audio and a synchronized spatial audio source.
- Representative Core assets and components, including animation and material
  feature-set/material-instance pairs.

Each generated backend should decode every fixture, validate its values, and
re-encode it byte-for-byte. At least one fixture should be authored by each
backend so the test does not rely on a single implementation as the encoder of
record.

Include negative fixtures for malformed discriminants, invalid booleans,
truncated arrays, excessive guarded lengths, and invalid semantic combinations.

## Not currently planned

Automated comparison between Core constants and Markdown registry tables is on
hold. It should not be added until the desired registry-maintenance workflow is
clear.
