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

Notes:
- `BufferView` references must target `Buffer` assets; higher-level assets reference buffers via views.
- Image/material tables are intentionally small (up to 16 entries) to align with typical sampler limits.
- Animation clips target entities directly; mesh assets can include joint/weight attributes and morph targets to support skinning and blendshapes.


## Components
|Name              | Identifier | Description |
|------------------|------------| ----------- |
| Global           | 0          | Marks entities with scene-wide effects that should be prioritized. |
| RPCEndpoint      | 1          | Advertises callable endpoints; argument hints are informational. |
| Billboard        | 2          | Rotates to face the viewer (full face or Y-axis only). |
| Environment      | 3          | Binds an environment asset with intensity/rotation. |
| Instance         | 4          | Describes instancing buffers and quantization ranges. |
| Interactive      | 5          | Marks entities as interactive. |
| CanTranslate     | 6          | Translation affordance with hard per-axis limits. |
| CanRotate        | 7          | Rotation affordance with hard per-axis limits. |
| CanScale         | 8          | Scale affordance with hard per-axis limits. |
| Activate         | 9          | Activation targets entities (with RPCEndpoint) to invoke. |
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

Notes:
- Component payloads are complete replacements; partial updates are not defined.
- `Global` marks entities whose effects may be scene-wide. Downstreams may prioritize their synchronization/UI.
- `RPCEndpoint` exposes callable entry points; arguments/results use `Any` encoding.

## Animation Notes

### Target Paths

Animation channels address component or asset members for time-varying value animations.

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

If a component id is not known, or the animation value cannot be coerced to the target component member, the node is free to ignore this animation.


### Value Coercion

It is possible, in this spec, to have an AnimationValueType that does not fully match with the target value type. The client may then coerce the animated value to the proper type. Legal conversions are provided:

| From (v)          | To (y)        | Notes         |
|---                |---            | ---           |
|f32                | Float3        | y = [v, v, v] |
|Float3             | f32           | y = v.x       |
|SRGBA8             | SRGB8         | y = [v.r, v.g, v.b] |
|SRGB8              | SRGBA8        | y = [v.r, v.g, v.b, 255] |

### Reference Evaluation

Example code for components. Similar approach is used for assets. Cocercion is not added in this sample.

```
function eval_animator(entity, animator, now_seconds):
    # Accumulate per target path:
    # overrides[path] = list of (value_type, weight, value)
    # additives[path] = list of (value_type, weight, value) in layer order
    overrides = Map<Path, List<Contribution>>()
    additives = Map<Path, List<Contribution>>()

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

        for channel in asset.channels:
            if channel.target.entity != entity:
                continue

            path = channel.target.path
            sampler_asset_id = asset.samplers[channel.sampler]
            sampler = resolve_anim_sampler_asset(sampler_asset_id)
            if sampler is missing:
                continue
            # Sample value at time clip_t (in seconds)
            value = sample_sampler(sampler, clip_t)  # returns typed value (Float/Float3/Rotation)

            c = Contribution(
                value_type = sampler.value_type,
                weight = max(layer.weight, 0),
                value = value,
                is_additive = (layer.is_additive != 0)
            )

            if c.is_additive:
                additives[path].push(c)   # preserves layer order naturally
            else:
                overrides[path].push(c)

    # Apply per-path evaluation:
    # First overrides (blended), then additives (applied in order).
    all_paths = union(keys(overrides), keys(additives))

    for path in all_paths:
        base = read_value(entity, path)   # current value in the component member

        # ----- Override stage -----
        if path in overrides:
            list = overrides[path]
            if list.size > 0:
                vt = list[0].value_type  # MUST match across contributors; otherwise ignore mismatches
                sum_w = Σ(ci.weight for ci in list)

                if sum_w > 0:
                    if vt == Float:
                        v = 0
                        for ci in list:
                            if ci.value_type != Float: continue
                            v += ci.weight * ci.value
                        base = v / sum_w

                    else if vt == Float3:
                        v = (0,0,0)
                        for ci in list:
                            if ci.value_type != Float3: continue
                            v += ci.weight * ci.value
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
        if path in additives:
            for ci in additives[path] (in stored order):
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

        write_value(entity, path, base)


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
