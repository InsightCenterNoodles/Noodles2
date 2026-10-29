# Well-Known Events

This document registers event contracts with common protocol-wide meanings.
They use the ordinary Core `EventRegistryComponent` and `EventMessage` wire
types; this registry does not introduce another extension or event namespace.

Individual additions to this registry are in the `well_known_events` directory.

Names beginning with `noodles.` are reserved for contracts registered here.
Applications SHOULD use another prefix for private events. Names are
case-sensitive UTF-8 byte strings. A producer using a registered name MUST use
its specified argument count, order, `Any` variants, and semantics.

The numeric `definition_identifier` remains local to each event registry and is
not assigned by this document. Consumers discover a well-known event by its
registered name and then use that definition's identifier when matching
`EventMessage.event_type`. A definition using a well-known name but a different
argument declaration is malformed and MUST NOT be interpreted as that
well-known event.

An incompatible change, including adding, removing, reordering, or changing an
argument, receives a new versioned name. Existing versioned contracts do not
change meaning. Consumers may ignore unknown names and versions.