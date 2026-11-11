# OpenThread Full Thread Device

A Full Thread Device (FTD, module `openthread_ftd`) has the router functions
of Thread: the first node forms the network and becomes its leader, the next
ones join it as children of a router, then become routers too. Each node of
this example:

- starts the Thread network of the `OPENTHREAD_*` settings at boot, see the
  [overview of the OpenThread examples](../README.md),
- prints its role when it changes,
- serves the name of its board over CoAP: `riot/board`, on the default CoAP
  port (5683),
- has the OpenThread CLI as the `ot` shell command (`openthread_cli_ftd`).

## Usage

Flash two nodes or more with the same settings:

```
make BOARD=<board> flash term
```

The first node becomes the leader of the network:

```
OpenThread FTD example
openthread: CoAP server on port 5683
> openthread: leader
```

The other ones join it, as children first, then as routers after up to two
minutes (the router selection jitter of Thread):

```
> openthread: child
> openthread: router
```

Get the mesh local address of a node with `ot ipaddr mleid`, and query its
CoAP server from another node with the CoAP client of the CLI. The CLI prints
the payload in hexadecimal: `6e61746976653634` is `native64`.

```
> ot ipaddr mleid
fdde:ad00:beef:0:4bf3:2234:1aab:ee85
Done
```

```
> ot coap get fdde:ad00:beef:0:4bf3:2234:1aab:ee85 riot/board
Done
> coap response from fdde:ad00:beef:0:4bf3:2234:1aab:ee85 with payload: 6e61746976653634
```

Other commands: `ot state`, `ot neighbor table`, `ot child table`,
`ot ping <address>`, `ot scan` or `ot help`.

## From a Linux host

With the [border router](../border_router/README.md) example, the nodes get an
address in the prefix of the host (`ot ipaddr`) and are reached from the host,
e.g. with `aiocoap-client` (aiocoap) or `coap-client` (libcoap):

```
aiocoap-client 'coap://[2001:db8::7b8:5565:a85:e0b4]/riot/board'
```
