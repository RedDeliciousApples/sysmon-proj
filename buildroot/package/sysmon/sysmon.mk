################################################################################
#
# sysmon
#
################################################################################

SYSMON_VERSION = main
SYSMON_SITE = $(BR2_EXTERNAL_SYSMON_PATH)/..
SYSMON_SITE_METHOD = local

define SYSMON_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) \
        CPPFLAGS="$(TARGET_CPPFLAGS) -Iinclude -Iexternal -MMD -MP" \
        CXXFLAGS="$(TARGET_CXXFLAGS) -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -pthread" \
        LDFLAGS="$(TARGET_LDFLAGS) -pthread"
endef

define SYSMON_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/build/sysmon \
        $(TARGET_DIR)/usr/bin/sysmon
endef

$(eval $(generic-package))