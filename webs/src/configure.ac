#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([webs], 1.0)
AM_INIT_AUTOMAKE([-Wall gnu foreign])
AC_CONFIG_SRCDIR([src/webs.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([netdb.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket strchr strerror strstr])

# Checks for libraries.
#PKG_CHECK_MODULES([DIAG], [diag])
#AC_SUBST([DIAG_CFLAGS])
#AC_SUBST([DIAG_LIBS])

#PKG_CHECK_MODULES([DSUTILS], [dsutils])
#AC_SUBST([DSUTILS_CFLAGS])
#AC_SUBST([DSUTILS_LIBS])

AS_CASE([$host],
    [arm*], [ARM=yes],
    [ARM=no]
)

#AM_CONDITIONAL(ARM, [test "x$ARM" = "xyes"])
AC_SUBST([CFLAGS])
AC_SUBST([CC])
#        application/Makefile \
AC_CONFIG_FILES([ \
        Makefile \
        src/Makefile \
        webs.pc
        ])
AC_OUTPUT
