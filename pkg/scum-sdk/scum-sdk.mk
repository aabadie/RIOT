MODULE = scum-sdk

CFLAGS += -Wno-sign-compare
CFLAGS += -Wno-type-limits
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-old-style-definition

SRC := \
  scm3c_hw_interface.c \
  optical.c \
  #

include $(RIOTBASE)/Makefile.base
