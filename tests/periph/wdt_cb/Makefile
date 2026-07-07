BOARD ?= nrf52840dk
include ../Makefile.periph_common

# The test relies on the callback message reaching the host before the
# watchdog resets the MCU, which depends on the platform warning period and
# the serial speed: keep a whitelist of boards known to work.
BOARDS_SUPPORTED = nrf52840dk nrf5340dk-app samr21-xpro sltb001a

FEATURES_REQUIRED += periph_wdt periph_wdt_cb

# On the nRF DKs the warning period is very short (2 cycles of the 32.768 kHz
# clock, ~61 us): raise the serial speed so enough of the callback message is
# transmitted before the reset. The on-board J-Link VCOM supports baudrates
# up to 1 MBd.
ifneq (,$(filter nrf52840dk nrf5340dk-app,$(BOARD)))
  BAUD ?= 460800
  CFLAGS += -DSTDIO_UART_BAUDRATE=$(BAUD)
endif

include $(RIOTBASE)/Makefile.include
