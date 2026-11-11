# OpenThread Minimal Thread Device

A Minimal Thread Device (MTD, module `openthread_mtd`) has no router
functions: it is an end device, the child of a router or of the leader of the
network (a Full Thread Device, e.g. the [ftd](../ftd/README.md) example), and
it is smaller than an FTD. Each node of this example:

- joins the Thread network of the `OPENTHREAD_*` settings at boot, see the
  [overview of the OpenThread examples](../README.md),
- is a sleepy end device: its radio is off between the polls of its parent,
  every `POLL_PERIOD_MS` milliseconds (1000), the parent keeps the frames for
  the child until its next poll; the timer behind `ZTIMER_USEC` only runs
  while it is used (`ztimer_ondemand_timer`),
- prints its role when it changes,
- serves the name of its board over CoAP: `riot/board`, on the default CoAP
  port (5683),
- has the OpenThread CLI as the `ot` shell command (`openthread_cli_mtd`).

## Usage

Flash an FTD, e.g. the [ftd](../ftd/README.md) or the
[border_router](../border_router/README.md) example, and an MTD with the same
settings:

```
make BOARD=<board> flash term
```

The MTD attaches to the FTD as a sleepy child:

```
OpenThread MTD example
openthread: sleepy end device, poll period 1000 ms
openthread: CoAP server on port 5683
> openthread: child
```

```
> ot mode
-
Done
> ot pollperiod
1000
Done
```

On its parent, `ot child table` lists the child with `R` (receiver on when
idle) at 0. A request to the child waits on the parent until the next poll of
the child, up to a poll period:

```
> ot coap get fdde:ad00:beef:0:ecc0:2a79:62f6:4e71 riot/board
Done
> coap response from fdde:ad00:beef:0:ecc0:2a79:62f6:4e71 with payload: 6e61746976653634
```

Build with `POLL_PERIOD_MS=0` for an end device with the radio always on, the
mode `r` instead of `-`.

## From a Linux host

With the [border router](../border_router/README.md) example, the nodes get an
address in the prefix of the host (`ot ipaddr`) and are reached from the host,
e.g. with `aiocoap-client` (aiocoap) or `coap-client` (libcoap):

```
aiocoap-client 'coap://[2001:db8::ecc0:2a79:62f6:4e71]/riot/board'
```
