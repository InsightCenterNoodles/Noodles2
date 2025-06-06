# Noodles2
Speculative NOODLES2 Specification

## Concepts
NOODLES2 is designed as a limited distributed entity component system protocol.

### Entity
These are just integers, clients should not rely on any kind of 

### Topology
In this network, we have a Node. A Node can connect to another Node (the Upstream) Others can connect to it (Downstream Nodes). At the moment, we are working with websockets. Possible libp2p options.

## Connection
Connection: A Node connects to another by sending an introduction message. Nothing is to be sent until the introduction. By default the Node that connects to another is considered to be a Downstream and the connectee is the Upstream. This relationship can be inverted through the use of an Authority flag that is sent in the introduction. Note that only one upstream is to be allowed normally. Thus, if two Nodes connect to a third and both request to be the upstream, it is up to the Node to decide who should be its upstream; the Node is free to close the connection(s) at will.

### Probing
A non-Node or prospective node may wish to obtain metadata about a Node. A 'probe' message may be sent instead of an introduction. A reply message should be sent and the connection should be closed on both sides.


### Session
Once connected, the session is then a series of messages. We have Transaction messages, which modify the ECS. These are to be translated to the client platform. We have RPC messages.

### Transaction
Modifies the ECS. 
These have an optional identifier.
