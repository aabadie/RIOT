BOARD ?= stm32f429i-disc1
include ../Makefile.tests_common

DISABLE_MODULE += test_utils_interactive_sync

USEMODULE += stmpe811
USEMODULE += touch_dev

USEMODULE += ztimer_msec
# Allow ztimer to sleep when the board provides an RTT
FEATURES_OPTIONAL += periph_rtt

include $(RIOTBASE)/Makefile.include
