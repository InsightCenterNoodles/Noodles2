# Core Extension

The Core extension defines the baseline assets and components required for interoperability. IDs are scoped under extension 0x0000 and should be available on every connection.

## Assets

|Name           | Identifier | Description |
|---------------|------------| ----------- |
| Buffer        |    0       | Raw byte storage, referenced by BufferView for higher-level assets.      |
| Environment   |    1       | Environment/sky lighting; references image table entries by index.       |
| Image         |    2       | Encoded image plus sampler/semantic flags.                               |
| Mesh          |    3       | Geometry attributes and indices in an uncompressed layout.               |
| PBRMaterial   |    4       | PBR surface parameters with optional extended features                   |
| Animation     |    5       | Clip referencing sampler assets and channels targeting entities/assets.  |
| AnimSampler   |    6       | Reusable keyframe sampler data (time/value arrays) referenced by Animation clips. |
| PBRMaterialFeatureSet | 7   | Immutable material shader/pipeline topology shared by parameter instances. |

Notes:
- `BufferView` references must target `Buffer` assets; higher-level assets reference buffers via views.
- Image/material tables are intentionally small (up to 16 entries) to align with typical sampler limits.
- Core clients must decode PNG and JPEG. Other listed image codecs and environment-rendering features are optional and may be ignored when unsupported.
- Meshes use right-handed coordinates, counterclockwise front faces, and glTF tangent handedness. Their bounding boxes describe the undeformed base pose.
- PBR color and emission textures are sRGB; roughness/metallic, occlusion, normal, and other non-color maps are linear. Unsupported extended material features may be ignored or approximated.
- `PBRMaterialFeatureSet` assets are immutable topology declarations. `PBRMaterial` assets are compact parameter instances; their optional extended block must agree structurally with the referenced feature set.
- Animation clips target entities directly; mesh assets can include joint/weight attributes and morph targets to support skinning and blendshapes.


## Components
|Name              | Identifier | Description |
|------------------|------------| ----------- |
| Global           | 0          | Marks entities with scene-wide effects. |
| RPCEndpoint      | 1          | Advertises callable endpoints; argument hints are informational. |
| Billboard        | 2          | Rotates to face the viewer (full face or Y-axis only). |
| Environment      | 3          | Binds an environment asset with intensity/rotation. |
| Instance         | 4          | Describes instancing buffers and quantization ranges. |
| Interactive      | 5          | Marks entities as interactive. |
| CanTranslate     | 6          | Translation affordance with hard per-axis limits. |
| CanRotate        | 7          | Rotation affordance with hard per-axis limits. |
| CanScale         | 8          | Scale affordance with hard per-axis limits. |
| Activate         | 9          | Primary and alternate RPC/Event actions for entity activation. |
| DirectionalLight | 10         | Directional light source. |
| PointLight       | 11         | Point light source. |
| SpotLight        | 12         | Spot light source. |
| PBRMaterial      | 13         | References a material asset. |
| Mesh             | 14         | References a mesh asset. |
| Name             | 15         | UTF-8 display name. |
| ChildOf          | 16         | Parent-child relationship. |
| CanSelect        | 17         | Entity is selectable. |
| IsSelected       | 18         | Selection state with outline color. |
| Text             | 19         | 3D text with color and height. |
| Transform        | 20         | Local transform (position/rotation/scale). |
| Visibility       | 21         | Visibility state (inherit/visible/invisible). |
| Animator         | 22         | Layered playback of animation clips with per-layer weights/looping. |
| Skin             | 23         | Joint palette and inverse bind matrices for skinned meshes. |
| MorphWeights     | 24         | Current morph target weights for an entity's mesh. |
| TimeSync         | 25         | Common timebase support.                           |
| ParameterRoot    | 26         | The start of a parameter UI tree.                    | 
| Parameter        | 27         | Common parameter storage. |
| ParamDefinition  | 28         | Parameter definition.                          | 
| ParamState       | 29         | Parameter state storage.                          | 
| EventRegistry    | 30         | Declares a registry-scoped namespace of transient broadcast events. |
| TransformSmoothingSettings | 31 | Interpolation delay and bounded extrapolation duration. |
| TransformSmoothingMode | 32 | Selects interpolation or sample-derived extrapolation. |
| TransformPositionSmoothing | 33 | Marks sampled position for smoothing. |
| TransformRotationSmoothing | 34 | Marks sampled rotation for smoothing. |
| TransformScaleSmoothing | 35 | Marks sampled scale for smoothing. |
| TransformSample | 36 | Timestamped complete local transform sample. |
| Description | 37 | Human-readable UTF-8 explanatory text. |
| SemanticTag | 38 | Whitespace-delimited stable semantic identifiers. |
| SemanticProperty | 39 | Semantic identifier to `Any` property values. |
| Relationship | 40 | Weak typed relationships to entities. |
| Participant | 41 | Marks a persistent known actor; does not imply presence. |
| Observation | 42 | Marks an observation, measurement, inference, or annotation. |
| ObservationSubject | 43 | Optional weak subject of an observation. |
| ObservationBounds | 44 | Optional local-space AABB for an observation region. |
| Task | 45 | Persistent task lifecycle state. |
| TaskResult | 46 | Optional task result content. |
| InteractionBounds | 47 | Optional local-space AABB for interaction picking. |

Notes:
- Component payloads are complete replacements; partial updates are not defined.
- `Global` marks entities whose effects may be scene-wide. See component documentation for more details.
- An absent transform is identity. Parent graphs are acyclic, and ancestor invisibility suppresses descendants.
- A `Global` environment applies to the whole scene. Producers should provide at most one; clients encountering several may select one. Non-global environments may be spatially blended at client discretion.
- Instance records are interleaved and entity-local. Skins require unique, live joints and exact inverse-bind matrix storage; finite morph weights may extend outside `[0,1]`.
- `Interactive` gates transform and activation affordances. Their limits are local to the parent; rotation is unrestricted or constrained to one parent-local axis relative to a reference orientation. Buttons and activation share an action union that invokes RPC endpoints or emits registered Events with arguments. Selection components represent shared authoritative selection; private client selection remains local.
- Parameter UI is an acyclic entity graph. Widget definition/state tags match, disabled state inherits to descendants, and numeric/option values obey their definitions.
- Event registries provide entity-scoped type IDs and informational argument hints for transient broadcasts. Event names and identifiers are unique within a registry.
- Transform samples are low-rate presentation hints in parent space. Full-support clients retain history and interpolate or derive bounded extrapolation; lightweight clients may snap to each newest sample. A hard `TransformComponent` update clears history and takes effect until a later ordered sample update. Reparenting or changing smoothing configuration also clears history. Initial snapshots preserve whether the hard transform or sample was updated last so late joiners reconstruct the same active source.
- Transform animation and transform sampling on one entity are strongly discouraged. Clients may resolve the conflict locally; ignoring sampling in favor of animation is recommended.
- Time synchronization is finite and monotonic within a session. Names are non-identifying display labels. Text uses local-unit height and sRGBA color; clients may render authored alpha as opaque.
- `RPCEndpoint` exposes callable entry points; arguments/results use `Any` encoding.
- Semantic tags use compact whitespace-delimited identifiers; semantic property
  keys are string identifiers. Relationships and observation subjects are weak:
  they may refer to absent or deleted entities and never keep targets alive.
  Semantic metadata does not inherit through the transform hierarchy.
- Participant entities describe known actors without identifying connections or
  asserting current presence. A Downstream may propose zero or many of them;
  connection establishment never implicitly creates one. Participant transform
  updates should remain low rate, with approximately 4 Hz or less recommended;
  clients may interpolate locally. Observations are marker entities with
  optional weak subjects and local bounds.
  Tasks retain their component through terminal states and may publish `Any`
  result content or relate result entities back to the task.
- Interaction bounds provide an optional entity-local AABB for picking without
  requiring clients to derive a region from rendered geometry.

### Relationship types

The entity carrying `RelationshipComponent` is the subject. Its relationship
entries use these Core-defined integer types; the target is the object.

| Name | Identifier | Meaning |
|------|------------|---------|
| RelatedTo | 0 | General association. |
| PartOf | 1 | Subject is a constituent or region of target. |
| Annotates | 2 | Subject annotates target. |
| CreatedBy | 3 | Subject was created by target. |
| DerivedFrom | 4 | Subject was derived from target. |
| ResultOf | 5 | Subject is a result of target. |

Future revisions may register additional values. An unknown value has no
implied behavior, but its weak target still does not constrain entity lifetime.
Relationships, including `CreatedBy`, are semantic assertions rather than
authenticated provenance. Upstream applies ordinary proposal policy but is not
required to verify that a sender controls or represents either endpoint.

## Animation Notes

### Target Paths

Animation channels address component or asset members for time-varying value animations.

An asset channel animates the shared logical asset. Every entity referencing
that asset observes the result. Per-entity material animation therefore requires
a distinct material asset identity for that entity.

Core-defined component targets:

| Component                 | `component_id`    | `member_index` → meaning    | `value_type` |
|---------------------------|-------------------|-----------------------------|--------------|
| `TransformComponent`      | (0, 20)           | 0 → `position`                | Float3    |
| `TransformComponent`      | (0, 20)           | 1 → `rotation`                | Rotation |
| `TransformComponent`      | (0, 20)           | 2 → `scale`                   | Float3 |
| `EnvironmentComponent`    | (0, 3)            | 1 → `intensity`               | Float | 
| `EnvironmentComponent`    | (0, 3)            | 2 → `rotation`                | Rotation |
| `DirectionalLightComponent` | (0, 10)         | 0 → `color`                   | SRGB8 |
| `DirectionalLightComponent` | (0, 10)         | 1 → `intensity`               | Float |
| `PointLightComponent`     | (0, 11)           | 0 → `color`                   | SRGB8 |
| `PointLightComponent`     | (0, 11)           | 1 → `intensity`               | Float |
| `SpotLightComponent`      | (0, 12)           | 0 → `color`                   | SRGB8 |
| `SpotLightComponent`      | (0, 12)           | 1 → `intensity`               | Float |
| `MorphWeightsComponent`   | (0, 24)           | 0 → weight[`sub_index`]       | Float |

Core-defined asset targets:

| Asset             | Asset Type ID | `member_index` → meaning    | `value_type` |
|-------------------|---------------|-----------------------------|--------------|
| `PBRMaterial`     | (0, 4)        | 0 →`base_color`              | SRGBA8 |
| `PBRMaterial`     | (0, 4)        | 1 →`roughness`               | Float |
| `PBRMaterial`     | (0, 4)        | 2 →`metallic`                | Float |
| `PBRMaterial`     | (0, 4)        | 3 →`ior`                     | Float |

If a target type or member is unknown, its referenced entity or asset does not exist, `sub_index` is invalid, or the sampled value cannot be coerced to the target member, the implementation MAY ignore that channel.


### Value Coercion

An `AnimationValueType` does not always exactly match the stored target type. An implementation MAY apply the following conversions:

| From (v)          | To (y)        | Notes         |
|---                |---            | ---           |
|f32                | Float3        | y = [v, v, v] |
|Float3             | f32           | y = v.x       |
|SRGBA8             | SRGB8         | y = [v.r, v.g, v.b] |
|SRGB8              | SRGBA8        | y = [v.r, v.g, v.b, 255] |

`SRGBA8` samples are decoded to linear RGB before interpolation and blending;
alpha is already linear. Results are converted back only when stored in an sRGB
target. CubicSpline interpolation is not valid for `SRGBA8` samplers.

### Reference Evaluation

The following pseudocode reflects the schema's separate `component_channels` and `asset_channels`. A target key contains every addressing field in the corresponding target pack. `read_target` converts stored values into their animation evaluation domain (including linear color), while `write_target` converts and clamps back to the stored representation. Other value coercion is omitted for clarity.

```
function eval_animator(animator, now_seconds):
    # Accumulate per component/asset target:
    # overrides[target] = list of (value_type, weight, value)
    # additives[target] = list of (value_type, weight, value) in layer order
    overrides = Map<TargetKey, List<Contribution>>()
    additives = Map<TargetKey, List<Contribution>>()

    for layer in animator.layers (in array order):
        if layer.weight <= 0:
            continue
        if layer.speed == 0 and layer.loop_mode != Clamp:
            # allowed, still evaluates; speed==0 just freezes clip time
            pass

        asset = resolve_animation_asset(layer.animation)
        if asset is missing:
            continue
        if asset.duration_seconds == 0:
            continue

        clip_t_raw = (now_seconds - layer.start_time) * layer.speed + layer.clip_time_offset
        clip_t = map_time(clip_t_raw, layer.loop_mode, asset.duration_seconds)

        for channel in asset.component_channels:
            target = ComponentTargetKey(
                channel.target.entity,
                channel.target.component_id,
                channel.target.member_index,
                channel.target.sub_index)
            accumulate_channel(asset, channel.sampler, target, layer, clip_t,
                               overrides, additives)

        for channel in asset.asset_channels:
            target = AssetTargetKey(
                channel.target.asset,
                channel.target.member_index,
                channel.target.sub_index)
            accumulate_channel(asset, channel.sampler, target, layer, clip_t,
                               overrides, additives)

    apply_contributions(overrides, additives)


function accumulate_channel(asset, sampler_index, target, layer, clip_t,
                            overrides, additives):
    if sampler_index >= asset.samplers.size:
        return

    sampler_asset_id = asset.samplers[sampler_index]
    sampler = resolve_anim_sampler_asset(sampler_asset_id)
    if sampler is missing:
        return
    # Sample value at time clip_t (in seconds)
    value = sample_sampler(sampler, clip_t)

    c = Contribution(
        value_type = sampler.value_type,
        weight = max(layer.weight, 0),
        value = value,
        is_additive = (layer.is_additive != 0)
    )

    if c.is_additive:
        additives[target].push(c)   # preserves layer order naturally
    else:
        overrides[target].push(c)


function apply_contributions(overrides, additives):

    # Apply per-target evaluation:
    # First overrides (blended), then additives (applied in order).
    all_targets = union(keys(overrides), keys(additives))

    for target in all_targets:
        base = read_target(target)

        # ----- Override stage -----
        if target in overrides:
            list = overrides[target]
            if list.size > 0:
                vt = list[0].value_type
                list = [ci for ci in list if ci.value_type == vt]
                sum_w = Σ(ci.weight for ci in list)

                if sum_w > 0:
                    if vt == Float:
                        v = 0
                        for ci in list:
                            v += ci.weight * ci.value
                        base = v / sum_w

                    else if vt == Float3:
                        v = (0,0,0)
                        for ci in list:
                            v += ci.weight * ci.value
                        base = v / sum_w

                    else if vt == SRGBA8:
                        v = (0,0,0,0)
                        for ci in list:
                            v += ci.weight * decode_srgba8_linear(ci.value)
                        base = v / sum_w

                    else if vt == Rotation:
                        # Weighted incremental slerp:
                        # Start from first rotation contributor (shortest-path is inside slerp).
                        q = first rotation contributor value
                        w_acc = weight of that contributor
                        for each subsequent rotation contributor ci:
                            w_new = ci.weight
                            if w_new <= 0: continue
                            t = w_new / (w_acc + w_new)
                            q = slerp(q, ci.value, t)
                            w_acc += w_new
                        base = normalize(q)

        # ----- Additive stage -----
        if target in additives:
            for ci in additives[target] (in stored order):
                if ci.weight <= 0:
                    continue

                if ci.value_type == Float:
                    base = base + ci.weight * ci.value

                else if ci.value_type == Float3:
                    base = base + ci.weight * ci.value

                else if ci.value_type == Rotation:
                    # Apply weighted delta from identity
                    delta = ci.value
                    base = base * slerp(identity_quat(), delta, clamp01(ci.weight))
                    base = normalize(base)

                else if ci.value_type == SRGBA8:
                    base = base + ci.weight * decode_srgba8_linear(ci.value)

        write_target(target, base)


function map_time(t, loop_mode, duration):
    if duration <= 0:
        return 0

    if loop_mode == Clamp:
        return clamp(t, 0, duration)

    if loop_mode == Loop:
        # handle negative robustly
        x = t mod duration
        if x < 0: x += duration
        return x

    if loop_mode == PingPong:
        period = 2 * duration
        x = t mod period
        if x < 0: x += period
        if x <= duration:
            return x
        else:
            return period - x



```
