# How to do proposed transactions

## Current design

- EntityID and AssetID are u64. Bit 63 is reserved for patch-local placeholders used only inside `ProposedChangeTransaction` payloads; canonical IDs must keep that bit clear.
- Proposals carry the same `ContentMessage` payloads as authoritative transactions. New entities/assets are identified with patch-local IDs and can be referenced elsewhere in the same proposal. Existing world content is always referenced with canonical IDs.
- Upstream processes proposals in arrival order, rejects attempts to delete assets or to reference unknown canonical IDs, and requires inline payloads for asset creation.
- Accepted proposals return `ProposedTransactionReply` with remap tables from patch-local → canonical entity/asset IDs; Downstreams should rewrite local state accordingly.

## Historical exploration (kept for context)

Downstream needs to use Entity and Asset IDs to refer to new content without collision with Upstream.

Options:
- Use a flag on entities and assets. Select a bit and use that as a provisional marker, disallow use by Upstream.
    - Painful; all IDs must be sanitized. All entities and assets need to be scanned and masked, or have extra logic to denote that a bit-set and bit-less entity are the same?
    - Removing a bit from entity IDs may push us to larger IDs, which bloats messages.
- Use Global Ids
    - Bloats all IDs (UUIDs?)
    - Or requires some way of creating negotiation-free id generation
- Use ID blocks
    - Upstream provides range of IDs that are reserved
    - Lots of bookkeeping for that; pool exhaustion, fragmentation, etc.
- Split
    - change proposals come in two forms, modify and create
    - modify is restricted to only using existing ids.
    - create works in a small ECS world that is unique to that transaction. the server merges that world in, remapping IDs, and then sends out a remap table to the requestor.
    - requires a two step dance if you ever want to mix new and existing ids (first create, then update). would this happen often?
    - requires a remap, meaning you have to keep state around.
    - server has to inspect all components with entities for remapping
- Larger id
    - go to 64 bit, with origin encoding
    - now we need node identity


REMAPPING IS ANNOYING. WE DONT WANT TO HAVE TO INSPECT ALL COMPONENTS.
- but do we have to anyway for security? possibly
- this complicates the implementations. we will need to have a remap(Component)

# Multiple Apps

> RESOLVED: NO MULTIPLE UPSTREAMS. you have one upstream, and anyone else who wants to join, has to be part of the tree.

NODES A, B, C, D

A -> C
B -> C
C -> D

C merges A and B

Pattern 3: realms / namespaces (client chooses where to write)
This is the more explicit version of Pattern 2.
Introduce a concept like realm or namespace:

Realm “A” → modifies/creates content in A’s authority domain.
Realm “B” → modifies/creates content in B’s domain.
Realm “C” → C’s own overlay domain.
Client D doesn’t just say “create entity”; it says:
text
Copy code
create_entity {
    realm: "A",
    payload: ...
}
Then C routes:
realm = "A" → C → A
realm = "B" → C → B
realm = "C" → C handles locally
ID-wise, this can be:
Realm corresponds to NodeID (if each realm == one node)
Or realm is a separate dimension and NodeID is still “the authority host”.
Pros:
No ambiguity: you always know who should own the new entity.
Works even when merging many upstreams.
Scales nicely as graph grows.
Cons:
API is a bit more explicit; D must know / choose realm.
You may want UX/UI or protocol defaults (“if realm omitted, use default”).

# Or

Just say the first server is the only one we shuffle data to.

# Or 

We leave it up to the client. it can pick the first, or just reject any changes. 

# Or

We disallow writes all together. all interaction has to be done through RPC?
- rpc has a single target, the server who created it. 
- but isnt this just like RPC, we just need to know what server created it?
- but what are we writing anyway?

if i write to A, no one who is connected to B will see it. 
