PKG_NAME=scum-sdk
PKG_URL=https://github.com/PisterLab/scum-sdk
PKG_VERSION=b62216830f75c316e847a44149b486eebf000a17
PKG_LICENSE=BSD

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/sdk/bsp -f $(CURDIR)/$(PKG_NAME).mk
