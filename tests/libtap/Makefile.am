noinst_LTLIBRARIES = libtap.la
libtap_la_SOURCES = tap.c tap.h test.h test.c
libtap_la_CFLAGS = -I$(top_srcdir)/src/ @GLIB_CFLAGS@
libtap_la_LIBADD = $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@
