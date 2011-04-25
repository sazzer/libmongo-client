noinst_PROGRAMS		= mongo-dump bson-inspect

AM_CFLAGS		= -I$(top_srcdir)/src/ @GLIB_CFLAGS@
LDADD			= $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@

mongo_dump_SOURCES	= mongo-dump.c
bson_inspect_SOURCES	= bson-inspect.c
