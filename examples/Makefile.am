noinst_PROGRAMS = mongo-dump

AM_CFLAGS = -I$(top_srcdir)/src/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@

mongo_dump_SOURCES = mongo-dump.c
