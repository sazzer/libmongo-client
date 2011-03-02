SUBDIRS		= libtap

bson_unit_tests = \
		unit/bson/bson_new \
		unit/bson/bson_empty \
		unit/bson/bson_string \
		unit/bson/bson_double \
		unit/bson/bson_boolean \
		unit/bson/bson_utc_datetime \
		unit/bson/bson_null \
		unit/bson/bson_int32 \
		unit/bson/bson_int64 \
		unit/bson/bson_regexp \
		unit/bson/bson_binary \
		unit/bson/bson_js_code \
		unit/bson/bson_symbol \
		unit/bson/bson_timestamp \
		unit/bson/bson_oid \
		unit/bson/bson_document \
		unit/bson/bson_array \
		\
		unit/bson/bson_reset \
		unit/bson/bson_new_from_data \
		\
		unit/bson/bson_build \
		unit/bson/bson_build_full \
		\
		unit/bson/bson_type_as_string

UNIT_TESTS = ${bson_unit_tests}

check_PROGRAMS = ${UNIT_TESTS}

check:
	@${MAKE} ${AM_MAKEFLAGS} check-UNIT

check-UNIT: ${UNIT_TESTS}
check-UNIT: TESTS+=${UNIT_TESTS}
check-UNIT: check-HARNESS

check-HARNESS: ${TESTS}
	$(AM_V_GEN)srcdir=${srcdir} prove -e '' ${PROVE_OPTIONS} ${TESTS}

.PHONY: check-UNIT check-HARNESS

AM_CFLAGS = -I$(top_srcdir)/src/ -I${top_srcdir}/tests/libtap/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la ${top_builddir}/tests/libtap/libtap.la @GLIB_LIBS@

EXTRA_DIST = README
