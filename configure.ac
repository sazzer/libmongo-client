dnl Process this file with autoconf to produce a configure script.
dnl
AC_INIT(src/bson.c)
AC_CONFIG_MACRO_DIR([m4])

dnl ***************************************************************************
dnl definitions

PACKAGE="libmongo-client"
VERSION="0"

dnl ***************************************************************************
dnl dependencies

GLIB_MIN_VERSION="2.10.0"

dnl ***************************************************************************
dnl Initial setup

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
AM_CONFIG_HEADER(config.h)

dnl ***************************************************************************
dnl Checks for programs.
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_CC_C99
if test "x$ac_cv_prog_cc_c99" = "xno"; then
	AC_MSG_ERROR("C99 standard compliant C compiler required. Try GCC 3.x or later.")
fi
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG
LT_INIT()

dnl ***************************************************************************
dnl How to do static linking?
dnl ***************************************************************************
AC_MSG_CHECKING(how to enable static linking for certain libraries)
ldversion=`ld -V 2>&1 | head -1`
if echo $ldversion | egrep "GNU|Solaris" > /dev/null; then
        LD_START_STATIC="-Wl,-Bstatic"
        LD_END_STATIC="-Wl,-Bdynamic"
        AC_MSG_RESULT(GNU or Solaris)
elif test $ostype = "HP-UX" > /dev/null; then
        LD_START_STATIC="-Wl,-a,archive"
        LD_END_STATIC="-Wl,-a,shared_archive"
        AC_MSG_RESULT(HP-UX)
elif test "$ostype" = "AIX"; then
        LD_START_STATIC="-Wl,-bstatic"
        LD_END_STATIC="-Wl,-bdynamic"
        AC_MSG_RESULT(AIX)
else
        LD_START_STATIC=""
        LD_END_STATIC=""
        AC_MSG_RESULT([no clues, linking everything dynamically, please send appropriate ld arguments to algernon@balabit.hu])
fi

dnl ***************************************************************************
dnl Miscellanneous headers
dnl ***************************************************************************

AC_HEADER_STDC

dnl ***************************************************************************
dnl Header checks
dnl ***************************************************************************

AC_CACHE_CHECK(for struct sockaddr_storage, blb_cv_c_struct_sockaddr_storage,
  [AC_EGREP_HEADER([sockaddr_storage], sys/socket.h, blb_cv_c_struct_sockaddr_storage=yes,blb_cv_c_struct_sockaddr_storage=no)])

if test "$blb_cv_c_struct_sockaddr_storage" = "yes"; then
	AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE,[1],[struct sockaddr_storage is present on your system])
fi

AC_CACHE_CHECK(for struct sockaddr_in6, blb_cv_c_struct_sockaddr_in6,
  [AC_EGREP_HEADER([sockaddr_in6], netinet/in.h, blb_cv_c_struct_sockaddr_in6=yes,blb_cv_c_struct_sockaddr_in6=no)])

dnl ***************************************************************************
dnl Checks for libraries
AC_CHECK_LIB(socket, socket, BASE_LIBS="$BASE_LIBS -lsocket")
AC_CHECK_LIB(nsl, gethostbyname, BASE_LIBS="$BASE_LIBS -lnsl")
AC_CHECK_LIB(resolv, res_init, RESOLV_LIBS="-lresolv")

AC_CHECK_FUNCS(inet_aton inet_ntoa getaddrinfo)

dnl ***************************************************************************
dnl GLib headers/libraries
dnl ***************************************************************************

GLIB_ADDONS="gmodule-2.0"
PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_MIN_VERSION $GLIB_ADDONS,,)

old_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$GLIB_CFLAGS"

AC_CACHE_CHECK(sanity checking Glib headers,
               blb_cv_glib_sane,
[AC_TRY_RUN([
#include <glib.h>

int main()
{
  if (sizeof(long) != GLIB_SIZEOF_LONG)
    return 1;
  return 0;
}
],
blb_cv_glib_sane=yes,
blb_cv_glib_sane=no,
blb_cv_glib_sane=yes)])
CPPFLAGS=$old_CPPFLAGS

if test "x$blb_cv_glib_sane" = "xno"; then
	AC_MSG_ERROR([Glib headers inconsistent with current compiler setting. You might be using 32 bit Glib with a 64 bit compiler, check PKG_CONFIG_PATH])
fi

dnl ***************************************************************************
dnl misc features to be enabled
dnl ***************************************************************************

AC_MSG_CHECKING(whether to enable IPv6 support)
if test "x$blb_cv_c_struct_sockaddr_in6" = "xyes"; then
	enable_ipv6=yes
	AC_MSG_RESULT(yes)
else
	enable_ipv6=no
	AC_MSG_RESULT(no)
fi

AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [package name])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [version number])

AC_OUTPUT(
	Makefile
	src/Makefile
	tests/Makefile
)
