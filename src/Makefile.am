LMC_CURRENT	= 0
LMC_REVISION	= 0
LMC_AGE		= 0

lib_LTLIBRARIES = libmongo-client.la
libmongo_client_la_LIBADD = @GLIB_LIBS@ @OPENSSL_LIBS@
libmongo_client_la_CFLAGS = @GLIB_CFLAGS@ @OPENSSL_CFLAGS@
libmongo_client_la_LDFLAGS = -version-info ${LMC_CURRENT}:${LMC_REVISION}:${LMC_AGE}

libmongo_client_la_SOURCES = \
	bson.c bson.h \
	mongo-wire.c mongo-wire.h \
	mongo-client.c mongo-client.h \
	mongo-utils.c mongo-utils.h \
	mongo-sync.c mongo-sync.h \
	mongo-sync-pool.c mongo-sync-pool.h \
	mongo.h \
	libmongo-private.h libmongo-macros.h

libmongo_client_includedir = $(includedir)/mongo-client
libmongo_client_include_HEADERS = \
	bson.h mongo-wire.h mongo-client.h \
	mongo-utils.h mongo-sync.h mongo-sync-pool.h mongo.h

pkgconfigdir            = $(libdir)/pkgconfig
pkgconfig_DATA          = libmongo-client.pc
