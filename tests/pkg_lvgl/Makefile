BOARD ?= stm32f429i-disc1
include ../Makefile.tests_common

# No interactive_sync
DISABLE_MODULE += test_utils_interactive_sync

USEPKG += lvgl
USEMODULE += lvgl_contrib
USEMODULE += ili9341

# Allow ztimer to sleep when the board provides an RTT
FEATURES_OPTIONAL += periph_rtt

CFLAGS += -DTHREAD_STACKSIZE_MAIN=2048

include $(RIOTBASE)/Makefile.include
