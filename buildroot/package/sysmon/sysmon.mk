################################################################################
#
# sysmon
#
################################################################################

SYSMON_VERSION = main
SYSMON_SITE = $(BR2_EXTERNAL_SYSMON_PATH)/..
SYSMON_SITE_METHOD = local

define SYSMON_BUILD_CMDS
    $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define SYSMON_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/build/sysmon \
        $(TARGET_DIR)/usr/bin/sysmon
endef

$(eval $(generic-package))