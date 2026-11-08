BOARD ?= stm32f429i-disc1
include ../Makefile.pkg_common

# No interactive_sync
DISABLE_MODULE += test_utils_interactive_sync

USEPKG += lvgl
USEMODULE += lvgl_contrib
USEMODULE += lvgl_widget_chart
USEMODULE += lvgl_widget_label
USEMODULE += lvgl_layout_flex
USEMODULE += lvgl_theme_default
USEMODULE += lvgl_theme_default_dark

include $(RIOTBASE)/Makefile.include

# SDL requires more stack
ifneq (,$(filter native native32 native64,$(BOARD)))
  CFLAGS += -DTHREAD_STACKSIZE_MAIN=64*1024
else ifneq (,$(filter esp%,$(CPU_FAM)))
  CFLAGS += -DTHREAD_STACKSIZE_MAIN=4*1024
else
  CFLAGS += -DTHREAD_STACKSIZE_MAIN=2*1024
endif
