APPLICATION = openthread_border_router

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# These are the boards that OpenThread stack has been tested on, with a serial
# port to the host
BOARDS_SUPPORTED := \
                    samr21-xpro \
                    iotlab-m3 \
                    iotlab-a8-m3 \
                    frdm-kw41z \
                    openlabs-kw41z-mini-256kib \
                    openlabs-kw41z-mini \
                    phynode-kw41z \
                    usb-kw41z \
                    cc2538dk \
                    remote-reva \
                    remote-revb \
                    omote \
                    openmote-cc2538 \
                    nrf52840dk \
                    nrf52840-mdk \
                    adafruit-feather-nrf52840-sense \
                    reel \
                    #

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../..

# the border router is a Full Thread Device
USEPKG += openthread
USEMODULE += openthread_ftd
USEMODULE += openthread_cli_ftd
# IPv6 to the host over ethos, stdio on ethos too
USEMODULE += openthread_border_router
USEMODULE += netdev_default

USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

# the Thread network of the nodes, e.g. the ftd and mtd examples
OPENTHREAD_PANID ?= 0xbeef
OPENTHREAD_CHANNEL ?= 26
CFLAGS += -DOPENTHREAD_PANID=$(OPENTHREAD_PANID)
CFLAGS += -DOPENTHREAD_CHANNEL=$(OPENTHREAD_CHANNEL)

# boards with 32 KiB of RAM: fewer OpenThread message buffers than the default
# of the FTD (44), the ethos buffers of the border router take the rest
ifneq (,$(filter samr21-xpro cc2538dk remote-reva remote-revb omote openmote-cc2538,$(BOARD)))
  CFLAGS += -DOPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS=24
endif

# the host side, as for gnrc_border_router: dist/tools/ethos/start_network.sh
# creates the tap interface, runs ethos and the UHCP server of the prefix
TAP ?= tap0
IPV6_PREFIX ?= 2001:db8::/64
ETHOS_BAUDRATE ?= 115200
CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)

TERMDEPS += host-tools
TERMPROG ?= sudo sh $(RIOTTOOLS)/ethos/start_network.sh
TERMFLAGS ?= $(PORT) $(TAP) $(IPV6_PREFIX) $(ETHOS_BAUDRATE)

DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

.PHONY: host-tools

host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)
