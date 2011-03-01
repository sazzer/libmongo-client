bson_unit_tests = unit/bson/bson_empty \
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
		unit/bson/bson_build \
		unit/bson/bson_build_full

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

AM_CFLAGS = -I$(top_srcdir)/src/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la @GLIB_LIBS@

EXTRA_DIST = README

tap_srcs = tap.h tap.c test.h test-generator.h test-generator.c

unit_bson_bson_empty_SOURCES = unit/bson/bson_empty.c ${tap_srcs}
unit_bson_bson_string_SOURCES = unit/bson/bson_string.c ${tap_srcs}
unit_bson_bson_double_SOURCES = unit/bson/bson_double.c ${tap_srcs}
unit_bson_bson_boolean_SOURCES = unit/bson/bson_boolean.c ${tap_srcs}
unit_bson_bson_utc_datetime_SOURCES = unit/bson/bson_utc_datetime.c ${tap_srcs}
unit_bson_bson_null_SOURCES = unit/bson/bson_null.c ${tap_srcs}
unit_bson_bson_int32_SOURCES = unit/bson/bson_int32.c ${tap_srcs}
unit_bson_bson_int64_SOURCES = unit/bson/bson_int64.c ${tap_srcs}
unit_bson_bson_regexp_SOURCES = unit/bson/bson_regexp.c ${tap_srcs}
unit_bson_bson_binary_SOURCES = unit/bson/bson_binary.c ${tap_srcs}
unit_bson_bson_js_code_SOURCES = unit/bson/bson_js_code.c ${tap_srcs}
unit_bson_bson_symbol_SOURCES = unit/bson/bson_symbol.c ${tap_srcs}
unit_bson_bson_timestamp_SOURCES = unit/bson/bson_timestamp.c ${tap_srcs}
unit_bson_bson_oid_SOURCES = unit/bson/bson_oid.c ${tap_srcs}
unit_bson_bson_document_SOURCES = unit/bson/bson_document.c ${tap_srcs}
unit_bson_bson_array_SOURCES = unit/bson/bson_array.c ${tap_srcs}

unit_bson_bson_build_SOURCES = unit/bson/bson_build.c ${tap_srcs}
unit_bson_bson_build_full_SOURCES = unit/bson/bson_build_full.c ${tap_srcs}
