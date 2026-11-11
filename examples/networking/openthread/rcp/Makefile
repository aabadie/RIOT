APPLICATION = openthread_rcp

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52840dk

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

# RCP: the Radio Co-Processor of an OpenThread host, Thread runs on the host
USEPKG += openthread
USEMODULE += openthread_rcp

# the radio of the board, a ZEP socket on native (dist/tools/zep_dispatch)
ifneq (,$(filter native native32 native64,$(BOARD)))
  USEMODULE += socket_zep
else
  USEMODULE += netdev_default
endif

# the UART of stdio carries the spinel link
USEMODULE += stdio_null

# baudrate of the spinel link, e.g. 500000 for the A8-M3 of IoT-LAB
OPENTHREAD_RCP_UART_BAUDRATE ?= 115200
CFLAGS += -DOPENTHREAD_RCP_UART_BAUDRATE=$(OPENTHREAD_RCP_UART_BAUDRATE)

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
