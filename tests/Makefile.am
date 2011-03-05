SUBDIRS		= libtap

bson_unit_tests	= \
		unit/bson/bson_new \
		unit/bson/bson_empty \
		\
		unit/bson/bson_append_string \
		unit/bson/bson_append_double \
		unit/bson/bson_append_boolean \
		unit/bson/bson_append_utc_datetime \
		unit/bson/bson_append_null \
		unit/bson/bson_append_int32 \
		unit/bson/bson_append_int64 \
		unit/bson/bson_append_regexp \
		unit/bson/bson_append_binary \
		unit/bson/bson_append_js_code \
		unit/bson/bson_append_symbol \
		unit/bson/bson_append_timestamp \
		unit/bson/bson_append_oid \
		unit/bson/bson_append_document \
		unit/bson/bson_append_array \
		\
		unit/bson/bson_reset \
		unit/bson/bson_new_from_data \
		\
		unit/bson/bson_build \
		unit/bson/bson_build_full \
		\
		unit/bson/bson_type_as_string \
		\
		unit/bson/bson_cursor_new \
		unit/bson/bson_find \
		unit/bson/bson_cursor_next \
		unit/bson/bson_cursor_type \
		unit/bson/bson_cursor_type_as_string \
		unit/bson/bson_cursor_key \
		\
		unit/bson/bson_cursor_get_string \
		unit/bson/bson_cursor_get_double \
		unit/bson/bson_cursor_get_document \
		unit/bson/bson_cursor_get_array \
		unit/bson/bson_cursor_get_binary \
		unit/bson/bson_cursor_get_oid \
		unit/bson/bson_cursor_get_boolean \
		unit/bson/bson_cursor_get_utc_datetime \
		unit/bson/bson_cursor_get_regex \
		unit/bson/bson_cursor_get_javascript \
		unit/bson/bson_cursor_get_symbol \
		unit/bson/bson_cursor_get_int32 \
		unit/bson/bson_cursor_get_timestamp \
		unit/bson/bson_cursor_get_int64

bson_func_tests	= \
		func/bson/huge_doc

mongo_utils_unit_tests	= \
		unit/mongo/utils/oid_init \
		unit/mongo/utils/oid_new \
		unit/mongo/utils/oid_new_with_time \
		unit/mongo/utils/parse_addr

mongo_wire_unit_tests	= \
		unit/mongo/wire/packet_new \
		unit/mongo/wire/packet_get_set_header \
		unit/mongo/wire/packet_get_set_header_raw \
		unit/mongo/wire/packet_get_set_data \
		\
		unit/mongo/wire/reply_packet_get_header \
		unit/mongo/wire/reply_packet_get_data \
		unit/mongo/wire/reply_packet_get_nth_document \
		\
		unit/mongo/wire/cmd_update \
		unit/mongo/wire/cmd_insert \
		unit/mongo/wire/cmd_query \
		unit/mongo/wire/cmd_get_more \
		unit/mongo/wire/cmd_delete \
		unit/mongo/wire/cmd_kill_cursors \
		unit/mongo/wire/cmd_custom

mongo_client_unit_tests	= \
		unit/mongo/client/connect \
		unit/mongo/client/packet_send \
		unit/mongo/client/packet_recv \
		unit/mongo/client/connection_get_requestid

UNIT_TESTS	= ${bson_unit_tests} ${mongo_utils_unit_tests} \
		${mongo_wire_unit_tests} ${mongo_client_unit_tests}
FUNC_TESTS	= ${bson_func_tests}
TESTCASES	= ${UNIT_TESTS} ${FUNC_TESTS}

check_PROGRAMS = ${TESTCASES}

AM_CFLAGS = -I$(top_srcdir)/src/ -I${top_srcdir}/tests/libtap/ @GLIB_CFLAGS@
LDADD = $(top_builddir)/src/libmongo-client.la ${top_builddir}/tests/libtap/libtap.la @GLIB_LIBS@

EXTRA_DIST = README

check:
	@${MAKE} ${AM_MAKEFLAGS} check-HARNESS

check-HARNESS: ${TESTS}
	$(AM_V_GEN)srcdir=${srcdir} prove -e '' ${PROVE_OPTIONS} ${TESTCASES}

.PHONY: check-HARNESS
