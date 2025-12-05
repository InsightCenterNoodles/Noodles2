Design notes:
- Early drafts allowed nodes to silently ignore unknown components. This breaks when entity remapping is required across multiple upstreams, so the protocol now relies on explicit capability exchange (shared extension sets) during the preamble.
- Entity remapping and richer capability negotiation are deferred to NOODLES3. For NOODLES2, keep topology small and prefer a single authoritative upstream.
- Components are deliberately narrow to avoid deltas; always send full component values.
