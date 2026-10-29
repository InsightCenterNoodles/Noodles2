
# `noodles.view.pose.v1`

Provides low-rate view-pose samples for uses such as time-and-motion studies,
session observation, and cooperative recording. It is telemetry, not a
high-rate remote-rendering or avatar-motion mechanism.

## Registry declaration

The `EventDefinition` has the exact UTF-8 name `noodles.view.pose.v1` and these
argument hints:

| Index | Name | `AnyTypeID` | Meaning |
|------:|------|-------------|---------|
| 0 | `sample_time` | `F64` | Estimated authority-monotonic sample time in seconds. |
| 1 | `position` | `Vec3` | View origin in scene world coordinates. |
| 2 | `rotation` | `UnitQuaternion` | Local-to-world view orientation. |

The corresponding `EventMessage.args` MUST contain exactly three values using
the `Any` variants corresponding to those types and in that order.

## Pose semantics

- `sample_time` MUST be finite and uses the monotonic authority-time estimate
  maintained from `TimeSyncComponent`. Samples from one registry entity MUST
  have strictly increasing sample times. A source without a usable time estimate
  SHOULD wait rather than substitute an unrelated local clock.
- `position` MUST contain finite values and uses the protocol's right-handed
  world coordinate system.
- `rotation` MUST contain finite values and be unit length within implementation
  tolerance. It rotates view-local axes into world space; local `-Z` is forward
  and local `+Y` is up. Consumers may normalize it.
- Projection, field of view, viewport size, gaze direction independent of head
  pose, and input-device poses are not part of this version.

The entity carrying the referenced `EventRegistryComponent` identifies the view
for the duration of that entity's lifetime. A source with multiple simultaneous
views uses a separate registry entity for each view. The entity may also carry a
`NameComponent` as a non-identifying display label. Its numeric entity ID is
session-local and must not be treated as a durable participant identity.

Event registration does not establish sender ownership. Any node is permitted
by the base event protocol to emit any registered event, and Upstream broadcasts
the event unchanged. Consequently, the registry entity is cooperative context,
not authenticated provenance. Upstream knows the originating connection while
receiving a downstream event, but recorder downstreams cannot recover that
connection identity from the broadcast event.

## Rate limit

A source MUST NOT emit this event for one registry entity more than once in any
250 millisecond interval (4 Hz). It MUST coalesce intermediate motion and emit
the newest sample when the next interval permits. It may sample less frequently,
emit only after meaningful movement, or stop emitting entirely. No heartbeat is
required.

Upstream may drop or rate-limit samples that violate this limit and may
disconnect a persistently violating sender under the general event resource
policy. Applications requiring smooth remote motion need a different state or
streaming facility rather than a higher rate for this event.

## Privacy and delivery

Pose reporting is optional even when a client implements the Core event system.
A client may decline to register or emit this event under user, application,
privacy, accessibility, or platform policy. Support MUST NOT be interpreted as
consent to tracking or recording.

The event is transient: it is not included in initial snapshots or replayed to
late joiners. Upstream policy, disconnection, or resource limits may cause gaps.
Recorders must tolerate missing samples and should handle pose data as
potentially sensitive information.
