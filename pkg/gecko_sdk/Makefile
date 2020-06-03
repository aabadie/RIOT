PKG_NAME=gecko_sdk
PKG_URL=https://github.com/basilfx/RIOT-gecko-sdk
PKG_VERSION=ec942b9f430193b5b3ddaf4cf2a85fc29e44696a
PKG_LICENSE=Zlib

include $(RIOTBASE)/pkg/pkg.mk

ifneq (llvm,$(TOOLCHAIN))
  CFLAGS += -Wno-int-in-bool-context
endif

all:
	"$(MAKE)" -C $(PKG_BUILDDIR)/dist
