check_PROGRAMS = test_bson_build_simple test_bson_build_complex

TESTS = run-tests.sh

AM_CFLAGS = -I$(top_srcdir)/src/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@

EXTRA_DIST = gen-known-good.py run-tests.sh \
	bson_build_simple.ok bson_build_complex.ok

test_srcs = test.c test.h

test_bson_build_simple_SOURCES = test_bson_build_simple.c $(test_srcs)
test_bson_build_complex_SOURCES = test_bson_build_complex.c $(test_srcs)

run-tests.sh: $(srcdir)/bson_build_simple.ok $(srcdir)/bson_build_complex.ok
$(srcdir)/bson_%.ok: $(srcdir)/gen-known-good.py
	python $(srcdir)/gen-known-good.py `basename $@ .ok` | sed -e "s,^'\(.*\)'$$,\1," >$@
