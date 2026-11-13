include ../Makefile.drivers_common

FEATURES_REQUIRED += periph_gpio_irq
# Select the bus: lis3dh_spi or lis3dh_i2c
DRIVER ?= lis3dh_spi
USEMODULE += $(DRIVER)
USEMODULE += xtimer

include $(RIOTBASE)/Makefile.include
