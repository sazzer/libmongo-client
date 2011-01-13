check_PROGRAMS = test_bson_build_base test_bson_build_compound \
	test_bson_reader_base test_bson_reader_iterator \
	\
	test_mongo_wire

TESTS = run-tests.sh test_bson_reader_base test_bson_reader_iterator \
	test_mongo_wire

AM_CFLAGS = -I$(top_srcdir)/src/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@

EXTRA_DIST = verify-bson.py run-tests.sh

test_srcs = test.c test.h test-generator.c test-generator.h

test_bson_build_base_SOURCES = test_bson_build_base.c $(test_srcs)
test_bson_build_compound_SOURCES = test_bson_build_compound.c $(test_srcs)

test_bson_reader_base_SOURCES = test_bson_reader_base.c $(test_srcs)
test_bson_reader_iterator_SOURCES = test_bson_reader_iterator.c $(test_srcs)

test_mongo_wire_SOURCES = test_mongo_wire.c $(test_srcs)

run-tests.sh: ${check_PROGRAMS} verify-bson.py
