# OpenThread Radio Co-Processor

A Radio Co-Processor (RCP, module `openthread_rcp`) is the radio of an
OpenThread host: Thread runs on the host, e.g. a Linux computer with the POSIX
platform of OpenThread, and the RCP only drives the radio. The host and the
RCP exchange spinel frames over HDLC on a UART. This is the setup of the
OpenThread border router [ot-br-posix](https://github.com/openthread/ot-br-posix)
(`otbr-agent`) and of the `ot-cli` and `ot-daemon` programs of OpenThread.

The spinel link is on the UART of stdio (`OPENTHREAD_RCP_UART_DEV`,
`UART_DEV(0)`), stdio is disabled (`stdio_null`). Its baudrate is
`OPENTHREAD_RCP_UART_BAUDRATE`: 115200 by default, e.g. 500000 for the
iotlab-m3 and iotlab-a8-m3 boards of IoT-LAB.

## Usage

Build and flash the RCP:

```
make BOARD=nrf52840dk flash
```

Build `ot-cli` for Linux from the sources of OpenThread, e.g. the sources of
the package, from the base directory of RIOT:

```
cmake -S build/pkg/openthread -B build/ot-posix -DOT_PLATFORM=posix -DOT_COAP=ON \
      -DOT_COMPILE_WARNING_AS_ERROR=OFF
make -C build/ot-posix ot-cli
```

Start it with the serial port of the board as radio:

```
build/ot-posix/src/posix/ot-cli 'spinel+hdlc+uart:///dev/ttyACM0?uart-baudrate=115200'
```

Then join the network of the [ftd](../ftd/README.md) and
[mtd](../mtd/README.md) examples, the defaults of the package:

```
> dataset clear
> dataset channel 26
> dataset panid 0xbeef
> dataset extpanid dead00beef00cafe
> dataset networkkey 00112233445566778899aabbccddeeff
> dataset networkname RIOT
> dataset meshlocalprefix fdde:ad00:beef:0::
> dataset commit active
> ifconfig up
> thread start
> state
child
Done
> coap start
Done
> coap get fdde:ad00:beef:0:4bf3:2234:1aab:ee85 riot/board
Done
> coap response from fdde:ad00:beef:0:4bf3:2234:1aab:ee85 with payload: 6e61746976653634
```

`ot-cli` keeps its settings, e.g. the active dataset, in `tmp/` of the
directory it runs from.

`otbr-agent` takes the same radio URL, e.g.
`otbr-agent -I wpan0 -B eth0 'spinel+hdlc+uart:///dev/ttyACM0?uart-baudrate=115200'`.

## Native

On native, the RCP uses a ZEP socket as radio, see the
[overview of the OpenThread examples](../README.md) for the ZEP dispatcher.
The host starts the RCP on a pseudo terminal, the UART of the RCP is this
terminal (`-c /dev/tty`). From the directory of the example:

```
make BOARD=native64
ot-cli 'spinel+hdlc+forkpty://bin/native64/openthread_rcp.elf?forkpty-arg=-z&forkpty-arg=[::1]:17754&forkpty-arg=-c&forkpty-arg=/dev/tty'
```
