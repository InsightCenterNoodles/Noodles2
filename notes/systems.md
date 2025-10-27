Use WIT for host interaction

Note: Exploratory notes. Mentions of CBOR reflect an older draft and do not
mirror the current little-endian binary wire format used by the headers.

Possible Interface:

```
package noodles:host

/// API provided by the host client to a guest WASM module
/// Used for ECS interaction, message passing, and visualization control

interface ecs {
  /// Adds a new component to an entity.
  add_component: func(entity: u32, component_type: string, data: list<u8>)

  /// Removes a component from an entity.
  remove_component: func(entity: u32, component_type: string)

  /// Emits a CBOR-encoded message to the upstream node.
  send_message: func(data: list<u8>)

  /// Schedules a timer event to call back into the module.
  set_timeout: func(id: u32, milliseconds: u64)

  /// Logs a debug message (for development purposes).
  log_debug: func(message: string)

  /// Requests the CBOR-encoded component data for an entity.
  /// Returns empty if not found.
  get_component: func(entity: u32, component_type: string) -> option<list<u8>>

  /// Spawns a new entity and returns its ID.
  spawn_entity: func() -> u32

  /// Despawns an entity and all its components.
  despawn_entity: func(entity: u32)
}

interface graphics {
  /// Attaches a mesh asset to an entity for visualization.
  attach_mesh: func(entity: u32, mesh_id: string)

  /// Attaches a material asset to an entity.
  attach_material: func(entity: u32, material_id: string)

  /// Sets the visibility of an entity.
  set_visibility: func(entity: u32, visible: bool)

  /// Updates the transform of an entity (CBOR-encoded transform component).
  set_transform: func(entity: u32, transform: list<u8>)
}

world noodles-host {
  import ecs
  import graphics
}


```
