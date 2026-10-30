# Well-Known Semantic Identifiers

This living registry assigns stable meanings to semantic identifiers used by
NOODLES2 components and discovery hints. It does not define another wire
extension. Implementations compare identifiers as exact UTF-8 token bytes after
the general malformed-text rule; they do not apply URI or Unicode
normalization.

Adding an identifier is backward compatible: clients that do not know it ignore
its meaning while retaining the containing state when possible. An assigned
identifier never changes meaning. An incompatible contract receives a new,
normally version-suffixed identifier.

Names beginning with `noodles.` are reserved for this document. Applications
SHOULD use URI-like identifiers or another non-reserved prefix for private
semantics.

## Relationship definitions

A relationship meaning is represented by an entity carrying
`RelationshipDefinitionComponent`. To claim a well-known meaning, that entity's
`SemanticTagComponent` contains the corresponding identifier. Its name,
description, properties, and other relationships may provide additional human
or machine-readable context. A definition SHOULD carry at most one
`noodles.relationship.*` identifier, though it may carry external vocabulary
identifiers describing the same meaning.

| Identifier | Directional meaning |
|------------|---------------------|
| `noodles.relationship.related_to` | Subject has a general association with target. |
| `noodles.relationship.part_of` | Subject is a constituent or region of target. |
| `noodles.relationship.annotates` | Subject annotates target. |
| `noodles.relationship.created_by` | Subject asserts that it was created by target. |
| `noodles.relationship.derived_from` | Subject asserts that it was derived from target. |
| `noodles.relationship.result_of` | Subject is a result entity belonging to target task. |

These relationships are semantic assertions rather than authenticated identity,
provenance, or causality. Upstream applies ordinary proposal policy but is not
required to verify either endpoint.

## Participant tags

No participant-kind identifiers are registered yet. Candidate contracts should
distinguish represented humans, autonomous agents, non-agent tools, instruments,
and other actors without implying current transport presence or authenticated
identity.

## Observation properties

No observation-property identifiers are registered yet. Expected future
contracts include value, unit, sample time, uncertainty, confidence, method,
and provenance. Contracts should define the exact `Any` type, units, coordinate
frame, and missing-value behavior rather than relying on a display name.

## Task properties

No task-property identifiers are registered yet. Expected future contracts
include normalized progress, status text, estimated cost, and structured error
information. Large or multiple task outputs should be result entities or assets
rather than large values embedded in a semantic property map.

