check_PROGRAMS = test_bson_build_base test_bson_build_compound \
	test_bson_reader_base test_bson_reader_iterator test_bson_from_data \
	\
	test_mongo_wire \
	\
	test_mongo_client \
	\
	test_oid \
	\
	test_invalid

TESTS = run-tests.sh \
	test_bson_reader_base test_bson_reader_iterator test_bson_from_data \
	test_mongo_wire \
	test_mongo_client \
	test_oid \
	test_invalid

AM_CFLAGS = -I$(top_srcdir)/src/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@

EXTRA_DIST = verify-bson.py run-tests.sh

test_srcs = test.c test.h test-generator.c test-generator.h

test_bson_build_base_SOURCES = test_bson_build_base.c $(test_srcs)
test_bson_build_compound_SOURCES = test_bson_build_compound.c $(test_srcs)

test_bson_reader_base_SOURCES = test_bson_reader_base.c $(test_srcs)
test_bson_reader_iterator_SOURCES = test_bson_reader_iterator.c $(test_srcs)
test_bson_from_data_SOURCES = test_bson_from_data.c $(test_srcs)

test_mongo_wire_SOURCES = test_mongo_wire.c $(test_srcs)

test_mongo_client_SOURCES = test_mongo_client.c $(test_srcs)

test_oid_SOURCES = test_oid.c $(test_srcs)

test_invalid_SOURCES = test_invalid.c $(test_srcs)

run-tests.sh: ${check_PROGRAMS} verify-bson.py
