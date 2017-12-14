include ../Makefile.tests_common

# include and auto-initialize all available sensors
USEMODULE += saul_default

USEMODULE += xtimer

SAUL_MODULES := $(shell find ${RIOTBASE}/sys/auto_init/saul -type f \
    \( ! -name "*auto_init_adc\.c" -and ! -name Makefile -and ! -name "*auto_init_gpio\.c" \) | \
		awk '{split($$0,a,"_"); print a[4] "_" a[5]}' | awk '{split($$0,a,"."); print a[1]}')

USEMODULE += ${SAUL_MODULES}

include $(RIOTBASE)/Makefile.include
