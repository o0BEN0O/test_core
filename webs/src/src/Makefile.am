#bin_PROGRAMS = webs
#webserver_SOURCES = webs.c webs.h

INC_DIR = -I../inc

MACRO = -DJRD_REDIRECT_COMMON_MACRO -DJRD_NEW_WEB_ARCH -DJRD_FEATURE_SIMPLE_DNS

AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -D_XOPEN_SOURCE \
        -D_GNU_SOURCE \
        -DLINUX \
        -DFEATURE_DATA_LINUX \
        -DDIGEST_ACCESS_SUPPORT \
        -DWEBS \
		$(INC_DIR)	\
		$(MACRO)

AM_CPPFLAGS = -D__packed__= \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -D_XOPEN_SOURCE \
        -D_GNU_SOURCE \
        -DLINUX \
        -DDIGEST_ACCESS_SUPPORT \
        -DWEBS

requiredlibs = -lpthread -ljson-c -lsock_client -ltimer -lsqlite3_db

library_includedir = $(pkgincludedir)

##server code
webs_sources = asp.c balloc.c mime64.c cgi.c default.c ejlex.c ejparse.c emfdb.c form.c h.c \
        handler.c md5c.c mime.c misc.c page.c ringq.c rom.c security.c sock.c sockGen.c sym.c \
        uemf.c um.c umui.c url.c value.c webrom.c webs.c websda.c websuemf.c webs_define.c win_web_api_pro.c \
        main.c

##

ext_sources = sock_clt.c queue.c jrd_md5.c
ext_h_sources = sock_clt.h queue.h jrd_md5.h
bin_PROGRAMS = webs
webs_SOURCES = $(webs_sources) $(ext_sources) $(webs_h_sources) $(ext_h_sources)
webs_LDADD = $(requiredlibs)
webs_CC = @CC@