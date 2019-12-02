include ../Makefile.tests_common

# Other available example: magic_wand
EXAMPLE ?= hello_world

USEPKG += tensorflow-lite
USEMODULE += tensorflow-lite-$(EXAMPLE)

DISABLE_MODULE += coxtexm-fpu

ifeq (magic_wand,$(EXAMPLE))
  USEMODULE += $(EXAMPLE)
  EXTERNAL_MODULE_DIRS += $(CURDIR)/$(EXAMPLE)
endif

include $(RIOTBASE)/Makefile.include
