APPLICATION = openthread_ftd

# If no BOARD is found in the environment, use this default:
BOARD ?= samr21-xpro

# These are the boards that OpenThread stack has been tested on
BOARDS_SUPPORTED := \
                    native \
                    native64 \
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

# FTD: a Full Thread Device has the router functions of Thread, it can be a
# router or the leader of the network
USEPKG += openthread
USEMODULE += openthread_ftd
# the OpenThread CLI as the ot shell command
USEMODULE += openthread_cli_ftd

# the radio of the board, a ZEP socket on native (dist/tools/zep_dispatch)
ifneq (,$(filter native native32 native64,$(BOARD)))
  USEMODULE += socket_zep
else
  USEMODULE += netdev_default
endif

USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

# the Thread network formed or joined at boot, the same on every node
OPENTHREAD_PANID ?= 0xbeef
OPENTHREAD_CHANNEL ?= 26
CFLAGS += -DOPENTHREAD_PANID=$(OPENTHREAD_PANID)
CFLAGS += -DOPENTHREAD_CHANNEL=$(OPENTHREAD_CHANNEL)

DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
