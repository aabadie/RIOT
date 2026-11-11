# OpenThread border router

The node is the border router of the Thread network (module
`openthread_border_router`): it connects the Thread network to a Linux host
over ethos (Ethernet over serial, on the serial port of stdio), as the
[gnrc_border_router](../../gnrc/border_router/README.md) example does for
GNRC, with the same host tools:

- the prefix of the host is requested with UHCP and published in the Thread
  network as an on-mesh prefix with SLAAC and a default route: the nodes get
  an address in it and route the datagrams to the host through the border
  router,
- the IPv6 datagrams are bridged between the Thread network and the host,
- the shell, with the OpenThread CLI as the `ot` command, is on the same
  serial link.

The border router is a Full Thread Device: it forms the network of the
`OPENTHREAD_*` settings, see the
[overview of the OpenThread examples](../README.md), or joins it.

## Usage

Build and flash the border router, then start the host side with `make term`.
`dist/tools/ethos/start_network.sh` (with sudo) creates the tap interface
(`TAP`, `tap0`), routes the prefix (`IPV6_PREFIX`, `2001:db8::/64`) to the
border router and runs ethos and the UHCP server of the prefix:

```
make BOARD=samr21-xpro flash term
```

```
OpenThread border router example
> openthread_br: prefix 2001:db8::/64 published
> ot netdata show
Prefixes:
2001:db8:0:0::/64 paros med 7c00
...
```

Then flash the nodes with the same settings, e.g. the [ftd](../ftd/README.md)
or [mtd](../mtd/README.md) example: they get an address in the prefix
(`ot ipaddr`) and the host reaches them, e.g. their CoAP server with
`aiocoap-client` (aiocoap) or `coap-client` (libcoap):

```
aiocoap-client 'coap://[2001:db8::7b8:5565:a85:e0b4]/riot/board'
```

The nodes reach the host at `fd00:dead:beef::1`, the address that
`start_network.sh` sets on the loopback interface of the host.

`ETHOS_BAUDRATE` (115200) must be the baudrate of the serial port of the
board, e.g. 500000 for the iotlab-m3 and iotlab-a8-m3 boards of IoT-LAB. On
IoT-LAB, `ethos_uhcpd.py` runs ethos on the SSH frontend, with a free tap
interface and a free prefix of the site:

```
make BOARD=iotlab-m3 ETHOS_BAUDRATE=500000 IOTLAB_NODE=m3-<id>.<site>.iot-lab.info flash
sudo ethos_uhcpd.py m3-<id> tap<n> <prefix>::/64
```

The boards with 32 KiB of RAM use 24 OpenThread message buffers instead of
the 44 of an FTD (`OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS`), the buffers of
ethos take the rest.
