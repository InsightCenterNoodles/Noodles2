# NOODLES2

**NREL Object Oriented Data Layout and Exploration System 2**

NOODLES2 defines a protocol for collaborative, distributed visualization of structured simulation data. It is based on the Entity-Component-System (ECS) architectural model and uses a compact, custom little-endian binary message format over WebSockets. The protocol is intended for real-time synchronization of 2D and 3D visual representations across networked clients, with a focus on extensibility, precision, and control over state consistency.

---

## Design Objectives

This specification is structured to achieve four primary objectives:

1. Utilize ECS as the core abstraction for representing application state, allowing structured, modular encoding of scene elements.
2. Maintain compact and efficient network communication via a simple, fixed/dynamic binary layout.
3. Establish a formal model of authority within the network topology to manage data consistency and conflict resolution.
4. Support scalable node hierarchies through a directed topology of Upstream and Downstream relationships.

The ECS model used in NOODLES2 decomposes application state into entities (identifiers), components (data attached to entities), and systems (behavioral logic applied to entity sets). This model is widely used in simulation and visualization engines for its ability to organize complex, dynamic environments in a declarative and scalable manner.

---

## Network Topology and Entity Structure

A NOODLES2 network consists of interconnected nodes. Each directed connection between two nodes assigns roles: the initiating node is the **Downstream**, and the receiving node is the **Upstream**. By default, the server-side node assumes the Upstream role, though this may be overridden during the handshake via an `authority` flag.

Each node may independently maintain one or more ECS worlds. An ECS world is a self-contained state model composed of entities, components, and optionally resources. Nodes may choose to forward, modify, or selectively suppress messages received from their Upstream when communicating with one or more Downstream connections.

Entity identifiers are 32-bit unsigned integers in this reference. Earlier drafts discussed 64-bit signed identifiers with sign indicating origin; that model is not used here. Nodes may implement additional isolation mechanisms—such as maintaining per-connection ECS instances—to avoid collisions or leakage of identifiers.

Components are attached to entities and convey application-specific semantics. All components are assigned numeric type identifiers. NOODLES2 requires that component updates be sent in their entirety; partial updates or deltas are not supported. To mitigate redundancy, design narrowly scoped components (e.g., separate position, rotation, and scale).

Resources are global data objects not bound to a specific entity. These are suitable for conveying shared configuration, input state, or environmental parameters.

Large, immutable data such as textures or geometry are transmitted via dedicated asset messages. Asset references are made using identifiers within component or resource payloads. This separation allows efficient caching and transfer of heavy payloads without affecting the primary message stream.

---

## Conflict Resolution and Update Semantics

NOODLES2 follows an authoritative model in which the Upstream node determines the canonical state. When a Downstream node proposes changes—such as creating an entity or modifying a component—it transmits a transaction proposal. The Upstream must then validate this request and respond with a `transaction_reply` message indicating acceptance or rejection.

All proposals are treated atomically. Partial acceptance is not permitted; any deviation from a complete accept results in a full rejection. Upon rejection, the Upstream is expected to broadcast the correct state for the affected entities or components. Implementations are recommended to use a last-write-wins strategy for resolving state contention, but this behavior is left undefined by the protocol and may be application-specific.

---

## Transport and Message Handling

NOODLES2 messages are transmitted over WebSockets using the binary layout described below. Upon connection, a node must issue one of two message types:

* `introduction`: initiates a full session and declares node intent
* `probe`: requests node metadata without participating in session state

Future revisions of the protocol may include support for `libp2p` as a transport layer, enabling more flexible and decentralized connection models. This is expected to facilitate features such as peer discovery, NAT traversal, and fully mesh-capable topologies.

---

## Architectural Principles

The NOODLES2 protocol emphasizes transparency, modularity, and clear contract boundaries. All updates are structured as complete values, avoiding ambiguity introduced by delta-based synchronization. Component definitions are kept granular to support efficient message transmission and simplify invalidation and replacement.

Simulation logic, input handling, and systems are not part of the protocol itself. Instead, NOODLES2 provides a consistent message and schema format for coordinating distributed state, leaving behavioral semantics to node implementations.

Message and type definitions are described directly in the header files in this repository. Legacy CDDL drafts are preserved under `old/` for reference only.

---

## Planned Extensions

A major planned feature is the introduction of **Systems**, which will formalize behavior and computation over ECS state. Systems will define operations applied to matching sets of entities and components and will support both reactive and scheduled modes of execution. This will transition NOODLES2 from a passive synchronization layer to a distributed simulation runtime.

Additional roadmap items include:

* Optional support for `libp2p` transport
* Runtime schema registration and negotiation
* Server-side asset deduplication
* Role-based stream filtering and access controls

These extensions are intended to broaden the protocol’s applicability and support larger-scale collaborative environments.

---

## Specification Status

This document describes a beta-stage protocol. The specification is not yet complete, and implementations are experimental. Structural elements and message definitions are subject to change as the design matures.

Feedback from early adopters and contributors is encouraged. Issues, proposed changes, and reference implementations are welcome.

---

## Wire Layout: Fixed & Dyn

To make parsing explicit and safe, message and struct types are split into two parts using a naming convention:

- FooFixed: memcpy-safe, trivially copyable fields (little-endian, C-style). No pointers or variable-length members.
- FooDyn: dynamic content parsed sequentially after FooFixed. May contain DynamicArray, DynamicMap, DynamicOptional, DynamicTaggedUnion, DynamicString, DynamicByteArray, OpaqueBytes, etc.

Wire layout is `[FooFixed][FooDyn…]` when both exist. Either part may be absent: some types are Fixed-only; others are Dyn-only.

Notes:
- There is no global size prefix (e.g., dyn_size) for the Dyn block. Many messages include per-field size hints in their Fixed part (e.g., `message_size`) to allow skipping unknown types safely.
- Fixed-only types carry all their data in FooFixed and can be copied directly from the wire.
- Dyn-only types start immediately with their dynamic content.
- The older in-source `// DYN` marker is deprecated and replaced by the explicit FooFixed/FooDyn types.

Examples in this repository:
- Handshake
  - Introduction: `IntroductionFixed` (Fixed-only; alias `Introduction`).
  - IntroductionReply: `IntroductionReplyFixed` + `IntroductionReplyDyn`.
  - Probe: `ProbeFixed` (Fixed-only) and `ProbeReplyFixed` + `ProbeReplyDyn`.
- Transactions and content
  - Transaction: `TransactionDyn` (Dyn-only).
  - NamedTransaction: `NamedTransactionFixed` + `NamedTransactionDyn`.
  - Content messages: e.g., `MsgModifyComponentFixed` + `MsgModifyComponentDyn`. Removal is via explicit delete messages (no negative-ID sentinels).
- Assets
  - Buffer: `BufferAssetFixed` + `BufferAssetDyn`.
  - Standard material: `StdMaterialAssetFixed` + `StdMaterialAssetDyn`.
  - Mesh: `UncompressedMeshFixed` + `UncompressedMeshDyn` (aliases `MeshAssetFixed`/`MeshAssetDyn`).
  - Large-asset streaming: `LargeAssetRequestFixed` and `LargeAssetReplyFixed` (Fixed-only), `LargeAssetChunkDyn` and `LargeAssetErrorDyn` (Dyn-only).
- Components
  - `ActivateComponentFixed` + `ActivateComponentDyn`.
  - Many are Fixed-only (e.g., `TransformComponent`, `NameComponent`, `VisibleComponent`).

Tagged unions:
- Dynamic values that vary by a discriminant are encoded as a tagged union using `DynamicTaggedUnion<Tag, Case<Tag, Tag::VALUE, Payload>...>`.
- On the wire, read the discriminant field first (named `type`), then parse the corresponding `Payload` listed by its `Case`.
- Unknown tags are currently an error unless the surrounding message provides a size bound to skip.
- Example: `BufferSource` maps `LARGE_ASSET => LargeAssetID`, `URL => DynamicString<u16>`, `INLINE => DynamicByteArray<u16>`.

Dynamic containers and optionals:
- `DynamicArray<N, T>`: `[count:N][T x count]` with elements laid out sequentially. For `DynamicString`, elements are bytes (no terminator).
- `DynamicMap<N, K, V>`: `[count:N][K0][V0][K1][V1]...` repeated `count` times.
- `DynamicOptional<T>`: presence/absence is controlled by an external discriminant (e.g., a flag bit in a Fixed header). Example: `StdMaterialAssetDyn.extended` is present when bit 10 is set in `StdMaterialAssetFixed.flags`.

Large assets:
- Request/Reply occurs on a secondary stream as negotiated during introduction. `LargeAssetReply` includes the total `asset_size`.
- One or more `LargeAssetChunk` messages follow. Completion is detected by accumulating chunk bytes to match `asset_size`.
- `LargeAssetError` terminates the transfer.

Static assertions for Fixed types:
- A macro `N2_STATIC_ASSERT_FIXED(T)` is defined in `common.hpp` and applied across headers to ensure all `*Fixed` types are `std::is_trivially_copyable_v<T>`.
- This provides a compile-time guarantee that Fixed parts are safe for direct wire copying.

Forward compatibility:
- A size prefix for Dyn (e.g., `dyn_size`) is not included yet. Once the layout stabilizes, adding it to FooFixed can bound FooDyn for skipping/forward-compat behaviors.
