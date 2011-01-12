check_PROGRAMS = test_bson_build_simple

TESTS = run-tests.sh

AM_CFLAGS = -I$(top_srcdir)/src/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@

test_srcs = test.c test.h

test_bson_build_simple_SOURCES = test_bson_build_simple.c $(test_srcs)
