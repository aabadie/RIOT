include ../Makefile.tests_common

USEPKG += lz4

# lz4 requires a custom thread stack size
CFLAGS += -DTHREAD_STACKSIZE_DEFAULT=16384

include $(RIOTBASE)/Makefile.include
