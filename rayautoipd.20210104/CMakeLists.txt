cmake_minimum_required (VERSION 2.6)
project (rayautoipd)

list(APPEND dhcpcd_C_SOURCES
    "dhcpcd/compat/arc4random.c"
    "dhcpcd/compat/arc4random_uniform.c"
    "dhcpcd/compat/pidfile.c"
    "dhcpcd/compat/reallocarray.c"
    "dhcpcd/compat/strlcpy.c"
    "dhcpcd/compat/strtoi.c"
    "dhcpcd/compat/strtou.c"
    "dhcpcd/compat/crypt/sha256.c"
    "dhcpcd/src/arp.c"
    "dhcpcd/src/bpf.c"
    "dhcpcd/src/common.c"
    "dhcpcd/src/control.c"
    "dhcpcd/src/dhcp.c"
    "dhcpcd/src/dhcp6.c"
    "dhcpcd/src/dhcp-common.c"
    "dhcpcd/src/dhcpcd.c"
    "dhcpcd/src/duid.c"
    "dhcpcd/src/eloop.c"
    "dhcpcd/src/if.c"
    "dhcpcd/src/if-linux.c"
    "dhcpcd/src/if-options.c"
    "dhcpcd/src/ipv4.c"
    "dhcpcd/src/ipv4ll.c"
    "dhcpcd/src/ipv6.c"
    "dhcpcd/src/ipv6nd.c"
    "dhcpcd/src/logerr.c"
    "dhcpcd/src/route.c"
    "dhcpcd/src/sa.c"
    "dhcpcd/src/script.c"
)

list(APPEND rayautoipd_C_SOURCES
    "raymarine/dhcpcd-embedded.c"
    "raymarine/ipv4ll-group.c"
)

include_directories (dhcpcd/ dhcpcd/src/ raymarine/)
add_executable (rayautoipd ${dhcpcd_C_SOURCES} ${rayautoipd_C_SOURCES})
install (TARGETS rayautoipd DESTINATION bin)

set (DHCPCD_C_FLAGS "-std=c99 -DHAVE_CONFIG_H -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -DINET -DARP -DARPING -DIPV4LL -DINET6 -DDHCP6")
set (CMAKE_C_FLAGS "-Wall -Wextra -ggdb ${DHCPCD_C_FLAGS} ${CMAKE_C_FLAGS}")
