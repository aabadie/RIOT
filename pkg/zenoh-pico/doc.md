@defgroup pkg_zenoh-pico  zenoh-pico
@ingroup  pkg
@brief    Zenoh protocol (pub/sub, queries) for constrained devices

[zenoh-pico](https://github.com/eclipse-zenoh/zenoh-pico) with a RIOT platform
layer (`ZENOH_GENERIC`): RIOT threads, mutexes and a scheduler based condition
variable, `ztimer64` clocks and TCP links over `sock_tcp`.

The build configuration is in `include/zenoh_generic_config.h`, every value
can be overridden with CFLAGS (for the package and the application). The
defaults target the client mode over TCP, e.g. towards a zenoh router
(`zenohd`) through a border router: locator `tcp/[2001:db8::1]:7447`, or
`tcp/[fe80::1%5]:7447` for a link local address of interface 5. Peer mode
needs `-DZ_FEATURE_UNICAST_PEER=1`, a listening socket accepts a single
connection.

With GNRC, reads use the `sock_tcp` timeouts. A write waits for the current
read timeout (`Z_CONFIG_SOCKET_TIMEOUT`, 100 ms) as GNRC TCP serializes the
calls on a connection. Closing a socket waits for the end of the TCP
connection: if the remote side doesn't close its side, e.g. a zenoh-pico peer
until its session is closed, the close waits for
`CONFIG_GNRC_TCP_CONNECTION_TIMEOUT_DURATION_MS` (120 s by default).

`tests/pkg/zenoh-pico` runs two sessions in peer mode on the IPv6 loopback
address: publication, query, and close of the listening session first.

License: EPL-2.0 OR Apache-2.0.
