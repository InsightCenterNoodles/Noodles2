# Noodles2
Speculative NOODLES2 Specification

## Concepts

### Topology
In this network, we have a Node. A node can connect to another Node (the Upstream) Others can connect to it (Downstream Nodes). 

Connection: A node connects to another by sending an introduction message. Nothing is to be sent until the introduction. By default the Node that connects to another is considered to be a Downstream and the connectee is the Upstream. This relationship can be inverted through the use of an Authority flag that is sent in the introduction.

### Entity

### 

## Design Notes

### Topology

There is a strong argument to making this a Star versus pure Distributed. There is typically a privledged node, or one that has access to things like the data, or the ground truth of the simulation. 

It does not make sense to try to replicate data to all nodes, without being transformed. It does not make sense for all nodes to do that transform. It could make sense for a subset of nodes to do that transform, but that does not mean they have to participate in the vis. 

Thus, we have a privledged node. In the absence of code distribution (and even with that), there is one sync point. We are not designing for thousands of connections at once. Other algorithms for distribution get hairy when it comes to consistency; we are expecting tens at the most, so why go through with that?


; If the authority flag is absent, the client sees the server as an Upstream. 
; The server then sends back with an introduction reply message.
; In the reply, the server may send information on the Asset Stream. See section for details.

; If the authority flag is present, the client is the Upstream to the server. This allows for role reversal.
; In this case, instead of introduction reply, the server will send to the client an introduction message, and the client will respond with the introduction reply
; - if the server does not wish to cede authority, it may close the channel
; - at the moment, we do not support multiple authorities. If the server is, or already has an authority, it may close the channel

; if a node receives a connection, and does not receive an introduction message or probe message in a reasonable time, it may close the channel.

; When you connect to an Upstream, you receive an initialization list.
; This list should point to assets that it knows about.
; So if you ask upstream about Asset 4, it knows that came from someone and 
; can relay

; Nodes may receive a 'probe' message. This message simply asks the node if it is active, and part of a session. If the node wishes, it may simply close the connection; this is a rejection of the request. Else, it should send a probe reply, with the name of the session, and an optional image that has the same semantics as the favicon for web. When the reply has been sent, the connection must be closed.