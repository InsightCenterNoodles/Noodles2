Components, assets, etc, are collected under Extensions. All extensions are given a 2 byte identifier (namespace) for components and assets. 

|Extension | Identifier | Description |
|----------|------------| ----------- |
| Core     | 0x0000     | Core required components. All implementations must support these.|

During connection Upstream and Downstream should only send messages related to shared Extensions