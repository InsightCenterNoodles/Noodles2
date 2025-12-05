Design notes:
- Early drafts allowed nodes to silently ignore unknown components. This breaks when entity remapping is required across multiple upstreams, so the protocol now relies on explicit capability exchange (shared extension sets) during the preamble.
- NOODLES2 now supports per-proposal remapping for created entities/assets (local IDs → canonical) but still keeps topology small and prefers a single authoritative upstream. Broader multi-upstream remapping and richer capability negotiation remain future work.
- Components are deliberately narrow to avoid deltas; always send full component values.
