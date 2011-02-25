lib_LTLIBRARIES = libmongo-client.la
libmongo_client_la_LIBADD = @GLIB_LIBS@ @OPENSSL_LIBS@
libmongo_client_la_CFLAGS = @GLIB_CFLAGS@ @OPENSSL_CFLAGS@

libmongo_client_la_SOURCES = \
	bson.c bson.h \
	mongo-wire.c mongo-wire.h \
	mongo-client.c mongo-client.h \
	mongo-utils.c mongo-utils.h \
	mongo-sync.c mongo-sync.h \
	mongo.h \
	libmongo-private.h

libmongo_client_includedir = $(includedir)/mongo-client
libmongo_client_include_HEADERS = \
	bson.h mongo-wire.h mongo-client.h \
	mongo-utils.h mongo-sync.h mongo.h

pkgconfigdir            = $(libdir)/pkgconfig
pkgconfig_DATA          = libmongo-client.pc
