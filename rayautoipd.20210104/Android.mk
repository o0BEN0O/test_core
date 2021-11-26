LOCAL_PATH:= $(call my-dir)
etc_dir := $(TARGET_OUT)/etc

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	dhcpcd/compat/arc4random.c \
	dhcpcd/compat/arc4random_uniform.c \
	dhcpcd/compat/pidfile.c \
	dhcpcd/compat/posix_spawn.c \
	dhcpcd/compat/reallocarray.c \
	dhcpcd/compat/strlcpy.c \
	dhcpcd/compat/strtoi.c \
	dhcpcd/compat/strtou.c \
	dhcpcd/compat/crypt/sha256.c \
	dhcpcd/src/arp.c \
	dhcpcd/src/bpf.c \
	dhcpcd/src/common.c \
	dhcpcd/src/control.c \
	dhcpcd/src/dhcp.c \
	dhcpcd/src/dhcp6.c \
	dhcpcd/src/dhcp-common.c \
	dhcpcd/src/dhcpcd.c \
	dhcpcd/src/duid.c \
	dhcpcd/src/eloop.c \
	dhcpcd/src/if.c \
	dhcpcd/src/if-linux.c \
	dhcpcd/src/if-options.c \
	dhcpcd/src/ipv4.c \
	dhcpcd/src/ipv4ll.c \
	dhcpcd/src/ipv6.c \
	dhcpcd/src/ipv6nd.c \
	dhcpcd/src/logerr.c \
	dhcpcd/src/route.c \
	dhcpcd/src/sa.c \
	dhcpcd/src/script.c \
	raymarine/dhcpcd-embedded.c \
	raymarine/ifaddrs.c \
	raymarine/ipv4ll-group.c

# As per CMakeLists.txt
LOCAL_C_INCLUDES += $(LOCAL_PATH)/dhcpcd
LOCAL_C_INCLUDES += $(LOCAL_PATH)/dhcpcd/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/raymarine
LOCAL_CFLAGS += -std=c99 -DHAVE_CONFIG_H -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -DINET -DARP -DARPING -DIPV4LL -DINET6 -DDHCP6

# Clang complains about configure.c's comparing array with null.
LOCAL_CLANG_CFLAGS := -Wno-error=duplicate-decl-specifier
LOCAL_SHARED_LIBRARIES := libc libcutils libnetutils
LOCAL_MODULE = rayautoipd
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := rayautoipd.axiom.assigned.cfg
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(etc_dir)
LOCAL_SRC_FILES := configs/rayautoipd.axiom.assigned.cfg
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := rayautoipd.axiom.unassigned.cfg
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(etc_dir)
LOCAL_SRC_FILES := configs/rayautoipd.axiom.unassigned.cfg
include $(BUILD_PREBUILT)
