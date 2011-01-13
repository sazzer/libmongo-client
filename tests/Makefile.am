check_PROGRAMS = test_bson_build_simple test_bson_build_complex \
	test_bson_reader_simple test_bson_reader_complex

TESTS = run-tests.sh test_bson_reader_simple test_bson_reader_complex

AM_CFLAGS = -I$(top_srcdir)/src/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@

EXTRA_DIST = verify-bson.py run-tests.sh

test_srcs = test.c test.h

test_bson_build_simple_SOURCES = test_bson_build_simple.c $(test_srcs)
test_bson_build_complex_SOURCES = test_bson_build_complex.c $(test_srcs)

test_bson_reader_simple_SOURCES = test_bson_reader_simple.c
test_bson_reader_complex_SOURCES = test_bson_reader_complex.c $(test_srcs)

run-tests.sh: ${check_PROGRAMS} verify-bson.py
