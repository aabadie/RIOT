PKG_NAME=openthread
PKG_URL=https://github.com/openthread/openthread.git
# v2026.11.0
PKG_VERSION=c6872259f4194fb76c7d333bda209343e8c88e71
PKG_LICENSE=BSD-3-Clause

include $(RIOTBASE)/pkg/pkg.mk

ifneq (,$(filter openthread_ftd,$(USEMODULE)))
  TD = ftd
  OT_FTD := ON
  OT_MTD := OFF
  OT_RCP := OFF
  OT_JOINER := OFF
else ifneq (,$(filter openthread_mtd,$(USEMODULE)))
  TD = mtd
  OT_FTD := OFF
  OT_MTD := ON
  OT_RCP := OFF
  OT_JOINER := ON
else ifneq (,$(filter openthread_rcp,$(USEMODULE)))
  TD = rcp
  OT_FTD := OFF
  OT_MTD := OFF
  OT_RCP := ON
  OT_JOINER := OFF
else
  $(error "Please use either USEMODULE=openthread_ftd, openthread_mtd or openthread_rcp")
endif

ifeq (rcp,$(TD))
  # radio only core and spinel over HDLC (Radio Co-Processor of a host)
  OT_CORE_LIB = openthread-radio
  OT_MODULES = openthread-radio openthread-rcp openthread-hdlc openthread-spinel-rcp
else
  OT_CORE_LIB = openthread-$(TD)
  OT_MODULES = openthread-$(TD)
  ifneq (,$(filter openthread_cli_$(TD),$(USEMODULE)))
    OT_MODULES += openthread-cli-$(TD)
  endif
endif
OT_MODULES += mbedcrypto mbedtls openthread-platform-utils-static
# X.509 certificates of the CoAP secure API (ECDHE-ECDSA cipher suite)
ifneq (,$(filter openthread_coaps,$(USEMODULE)))
  OT_MODULES += mbedx509
endif

# Enable CoAP
OT_COAP ?= ON
# Enable the CoAP secure API (CoAP over DTLS) with openthread_coaps
OT_COAPS ?= $(if $(filter openthread_coaps,$(USEMODULE)),ON,OFF)
# Enable the border router API with openthread_border_router
OT_BORDER_ROUTER ?= $(if $(filter openthread_border_router,$(USEMODULE)),ON,OFF)
# Enable SLAAC: addresses in the on-mesh prefixes of the border routers
OT_SLAAC ?= ON
# Enable the ping sender (CLI ping)
OT_PING_SENDER ?= ON

OT_PLATFORM_DEFINES = OPENTHREAD_SETTINGS_RAM=1
# cmake list
OT_EMPTY :=
OT_SPACE := $(OT_EMPTY) $(OT_EMPTY)

OT_C_FLAGS = $(CFLAGS_CPU)
# OpenThread is built with the macros of the application (riotbuild.h): the
# OPENTHREAD_CONFIG_* values of its CFLAGS take precedence over the ones of
# include/platform_config.h and of OpenThread
OT_C_FLAGS += -include $(RIOTBUILD_CONFIG_HEADER_C)
OT_C_FLAGS += -fdata-sections
OT_C_FLAGS += -ffunction-sections -Os
OT_C_FLAGS += -fshort-enums
OT_C_FLAGS += -Wno-implicit-fallthrough
OT_C_FLAGS += -Wno-unused-parameter
OT_CXXFLAGS += -Wno-class-memaccess
OT_CXXFLAGS += -DOPENTHREAD_TARGET_RIOT


# build directory of each library
OT_LIB_DIR_openthread-ftd = src/core
OT_LIB_DIR_openthread-mtd = src/core
OT_LIB_DIR_openthread-radio = src/core
OT_LIB_DIR_openthread-cli-ftd = src/cli
OT_LIB_DIR_openthread-cli-mtd = src/cli
OT_LIB_DIR_openthread-rcp = src/ncp
OT_LIB_DIR_openthread-hdlc = src/lib/hdlc
OT_LIB_DIR_openthread-spinel-rcp = src/lib/spinel
OT_LIB_DIR_openthread-platform-utils-static = examples/platforms/utils
OT_LIB_DIR_mbedtls = third_party/mbedtls/repo/library
OT_LIB_DIR_mbedcrypto = third_party/mbedtls/repo/library
OT_LIB_DIR_mbedx509 = third_party/mbedtls/repo/library

ifeq (llvm,$(TOOLCHAIN))
  OT_C_FLAGS += -target $(TARGET_ARCH_LLVM)
  OT_C_FLAGS += $(GCC_C_INCLUDES)
  OT_CXXFLAGS += $(GCC_CXX_INCLUDES)
endif

# OpenThread is built with an external platform: the platform layer is in
# contrib/, the applications of OpenThread (cli, ncp, rcp) are not built.
OT_CMAKE_OPTIONS = \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DCMAKE_C_COMPILER="$(CC)" \
  -DCMAKE_C_COMPILER_AR="$(AR)" \
  -DCMAKE_C_COMPILER_RANLIB="$(RANLIB)" \
  -DCMAKE_C_FLAGS="$(OT_C_FLAGS)" \
  -DCMAKE_CXX_COMPILER="$(CXX)" \
  -DCMAKE_CXX_FLAGS="$(OT_C_FLAGS) $(OT_CXXFLAGS) -fno-exceptions -fno-rtti" \
  -DCMAKE_NM="$(NM)" \
  -DCMAKE_STRIP="$(STRIP)" \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DOT_PLATFORM=external \
  -DOT_PLATFORM_CONFIG="$(RIOTBASE)/pkg/openthread/include/platform_config.h" \
  -DOT_APP_CLI=OFF \
  -DOT_APP_NCP=OFF \
  -DOT_APP_RCP=OFF \
  -DOT_FTD=$(OT_FTD) \
  -DOT_MTD=$(OT_MTD) \
  -DOT_RCP=$(OT_RCP) \
  -DOT_JOINER=$(OT_JOINER) \
  -DOT_COAP=$(OT_COAP) \
  -DOT_COAPS=$(OT_COAPS) \
  -DOT_BORDER_ROUTER=$(OT_BORDER_ROUTER) \
  -DOT_PING_SENDER=$(OT_PING_SENDER) \
  -DOT_SLAAC=$(OT_SLAAC) \
  -DOT_TCP=OFF \
  -DOT_PLATFORM_DEFINES="$(subst $(OT_SPACE),;,$(strip $(OT_PLATFORM_DEFINES)))"

# the options and the source directory of the build: it is configured again
# when they change (another RIOTBASE needs a new cmake cache)
OT_CMAKE_OPTIONS_FILE = $(PKG_BUILD_DIR)/riot-cmake-options

# the build of OpenThread runs every time: it rebuilds what changed (options,
# headers of RIOT), the archives are copied when they changed
all: $(PKG_BUILD_DIR)/Makefile
	$(QQ)"$(MAKE)" -C $(PKG_BUILD_DIR) $(OT_MODULES)
	$(Q)$(foreach m,$(OT_MODULES),cmp -s $(PKG_BUILD_DIR)/$(OT_LIB_DIR_$(m))/lib$(m).a $(BINDIR)/$(m).a \
	  || cp $(PKG_BUILD_DIR)/$(OT_LIB_DIR_$(m))/lib$(m).a $(BINDIR)/$(m).a;)

# mbedtls is a git submodule of OpenThread, fetched at the revision pinned by
# OpenThread
OT_MBEDTLS_DIR = $(PKG_SOURCE_DIR)/third_party/mbedtls/repo

$(OT_MBEDTLS_DIR)/CMakeLists.txt: $(PKG_PREPARED)
	$(Q)$(GIT_IN_PKG) submodule update $(GIT_QUIET) --init --recursive --depth 1 third_party/mbedtls/repo

$(OT_CMAKE_OPTIONS_FILE): FORCE
	$(Q)mkdir -p $(PKG_BUILD_DIR)
	$(Q)echo '-H$(PKG_SOURCE_DIR) $(OT_CMAKE_OPTIONS)' | '$(LAZYSPONGE)' $(LAZYSPONGE_FLAGS) '$@'

$(PKG_BUILD_DIR)/Makefile: $(OT_MBEDTLS_DIR)/CMakeLists.txt $(OT_CMAKE_OPTIONS_FILE)
	$(Q)rm -f $(PKG_BUILD_DIR)/CMakeCache.txt
	cmake -Wno-dev -B$(PKG_BUILD_DIR) -H$(PKG_SOURCE_DIR) $(OT_CMAKE_OPTIONS)
