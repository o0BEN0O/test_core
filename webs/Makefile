include $(TOPDIR)/rules.mk

PKG_NAME:=webs
PKG_VERSION:=2013-09-03
PKG_RELEASE:=1
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

PKG_FIXUP:=autoreconf
PKG_INSTALL=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=oem
  CATEGORY:=OEM
  DEPENDS:=+libjson-c +common +sqlite3-db +timer +sock-client
  TITLE:=Jrd comunicattion web server
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(CP) $(PKG_INSTALL_DIR)/*  $(1)/
endef

define Package/$(PKG_NAME)/install
	$(CP) $(PKG_INSTALL_DIR)/*  $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
