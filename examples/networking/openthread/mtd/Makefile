APPLICATION = openthread_mtd

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

# MTD: a Minimal Thread Device has no router functions, it is an end device,
# the child of a router (an FTD)
USEPKG += openthread
USEMODULE += openthread_mtd
# the OpenThread CLI as the ot shell command
USEMODULE += openthread_cli_mtd

# the radio of the board, a ZEP socket on native (dist/tools/zep_dispatch)
ifneq (,$(filter native native32 native64,$(BOARD)))
  USEMODULE += socket_zep
else
  USEMODULE += netdev_default
endif

USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

# the Thread network joined at boot, the same on every node
OPENTHREAD_PANID ?= 0xbeef
OPENTHREAD_CHANNEL ?= 26
CFLAGS += -DOPENTHREAD_PANID=$(OPENTHREAD_PANID)
CFLAGS += -DOPENTHREAD_CHANNEL=$(OPENTHREAD_CHANNEL)

# sleepy end device: the radio is off between the polls of the parent, every
# POLL_PERIOD_MS milliseconds. 0 for an end device with the radio always on.
POLL_PERIOD_MS ?= 1000
CFLAGS += -DPOLL_PERIOD_MS=$(POLL_PERIOD_MS)

# the timer behind ZTIMER_USEC only runs while it is used, e.g. during the
# CSMA-CA backoffs of OpenThread on the radios without CSMA-CA in hardware
USEMODULE += ztimer_ondemand_timer

DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
