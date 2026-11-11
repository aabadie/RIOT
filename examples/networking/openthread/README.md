# OpenThread examples

[OpenThread](https://openthread.io) is an open source implementation of
[Thread](https://www.threadgroup.org), an IPv6 mesh network over IEEE 802.15.4
and the network layer of Matter. The `openthread` package of RIOT runs it on
the IEEE 802.15.4 Radio HAL of RIOT.

| Example | Description |
|---------|-------------|
| [ftd](./ftd/README.md) | Full Thread Device: router or leader of the network, with a CoAP server. |
| [mtd](./mtd/README.md) | Minimal Thread Device: sleepy end device, with a CoAP server. |
| [border_router](./border_router/README.md) | Border router between the Thread network and a Linux host, over ethos. |
| [rcp](./rcp/README.md) | Radio Co-Processor of OpenThread on a Linux host (`ot-cli`, `otbr-agent`). |

## The Thread network

The devices of a Thread network share its settings. The ftd, mtd and
border_router examples start the network at boot from the `OPENTHREAD_*`
macros of the package (`pkg/openthread/include/ot.h`): the Makefiles set the
channel and the PAN ID, the other settings are the defaults of the package.

| Setting           | Default                            | Macro                          |
|-------------------|------------------------------------|--------------------------------|
| Channel           | `26`                               | `OPENTHREAD_CHANNEL`           |
| PAN ID            | `0xbeef`                           | `OPENTHREAD_PANID`             |
| Extended PAN ID   | `dead00beef00cafe`                 | `OPENTHREAD_EXT_PANID`         |
| Network key       | `00112233445566778899aabbccddeeff` | `OPENTHREAD_NETWORK_KEY`       |
| Network name      | `RIOT`                             | `OPENTHREAD_NETWORK_NAME`      |
| Mesh local prefix | `fdde:ad00:beef::/64`              | `OPENTHREAD_MESH_LOCAL_PREFIX` |

The nodes built with the same values form one network. Choose another channel
and PAN ID to avoid the networks of other users:

```
make BOARD=<board> OPENTHREAD_CHANNEL=17 OPENTHREAD_PANID=0x2b1f flash
```

The default network key is public: set your own one in the CFLAGS of the
application for anything else than tests.

## The OpenThread CLI

The `ot` shell command of the ftd, mtd and border_router examples is the
[CLI of OpenThread](https://github.com/openthread/openthread/blob/main/src/cli/README.md),
e.g. `ot state`, `ot ipaddr`, `ot ping <address>`, `ot coap get <address> <path>`
or `ot help`.

The application calls the OpenThread API in the thread of OpenThread: in the
handler of an event posted to the event queue of the package
(`openthread_get_evq()`), see the `main.c` of the ftd and mtd examples.

## Native

On the native boards, the nodes use a ZEP socket as radio (`socket_zep`).
Start the ZEP dispatcher, then each node in its own terminal:

```
make -C dist/tools/zep_dispatch
dist/tools/zep_dispatch/bin/zep_dispatch ::1 17754
```

```
make -C examples/networking/openthread/ftd BOARD=native64
examples/networking/openthread/ftd/bin/native64/openthread_ftd.elf -z [::1]:17754
```
