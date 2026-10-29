# Audio Extension

Audio is optional extension `0x0001` and depends on Core (`0x0000`). It provides
buffer-backed audio clips and synchronized spatial point sources. A peer that
does not implement audio does not advertise this extension.

## Assets

| Name | Identifier | Description |
|------|------------|-------------|
| Audio | 0 | Encoded audio held by a Core `BufferView`. |

## Components

| Name | Identifier | Description |
|------|------------|-------------|
| AudioSource | 0 | Synchronized omnidirectional point source attached to an entity. |

`AudioAsset` format is detected from its bytes. PCM16 RIFF/WAVE with one or two
channels is the baseline; other codecs and layouts are optional. Spatialization
uses a mono signal, with baseline stereo content downmixed before playback.

`AudioSourceComponent` anchors its playhead to authority time and uses linear
distance attenuation between its full-gain and silent distances. The listener
belongs to each downstream's local view state. Visibility, source rotation, and
source scale do not affect sound. Directional cones, Doppler, filters, reverb,
and authoritative listener state are outside this extension.

Clients retain final control over audio output and may mute, defer, limit, or
reject playback according to local policy.
