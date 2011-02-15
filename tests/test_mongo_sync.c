#include "test.h"
#include "test-generator.h"

#include "mongo.h"

#include <glib.h>

void
test_mongo_sync_cmd_insert (void)
{
  bson *doc;
  mongo_connection *conn;

  TEST (mongo_sync.cmd_insert);
  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  g_assert (conn);

  doc = test_bson_generate_nested ();

  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc));
  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc));

  bson_free (doc);

  g_assert_cmpint (mongo_connection_get_requestid (conn), ==, 2);

  mongo_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_update (void)
{
  bson *sel, *upd;
  mongo_connection *conn;
  guint8 *oid;

  TEST (mongo_sync.cmd_update);
  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  g_assert (conn);

  sel = bson_new ();
  oid = mongo_util_oid_new (1);
  bson_append_oid (sel, "_id", oid);
  g_free (oid);
  bson_finish (sel);

  upd = test_bson_generate_nested ();

  g_assert (mongo_sync_cmd_update (conn, TEST_SERVER_NS,
				   MONGO_WIRE_FLAG_UPDATE_UPSERT,
				   sel, upd));
  g_assert (mongo_sync_cmd_update (conn, TEST_SERVER_NS,
				   MONGO_WIRE_FLAG_UPDATE_UPSERT,
				   sel, upd));
  bson_free (sel);
  bson_free (upd);

  g_assert_cmpint (mongo_connection_get_requestid (conn), ==, 2);

  mongo_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_query (void)
{
  bson *doc, *sel;
  mongo_connection *conn;
  mongo_packet *p;

  bson_cursor *c;
  mongo_reply_packet_header rh;
  gint32 i;

  TEST (mongo_sync.cmd_query);
  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  g_assert (conn);

  doc = test_bson_generate_flat ();
  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc));
  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc));
  bson_free (doc);
  g_assert_cmpint (mongo_connection_get_requestid (conn), ==, 2);

  sel = bson_new ();
  bson_append_int32 (sel, "int32", 1984);
  bson_finish (sel);
  g_assert ((p = mongo_sync_cmd_query (conn, TEST_SERVER_NS,
				       0, 0, 1, sel, NULL)) != NULL);
  g_assert (mongo_wire_reply_packet_get_header (p, &rh));
  g_assert_cmpint (rh.start, ==, 0);
  g_assert_cmpint (rh.returned, ==, 1);
  bson_free (sel);

  g_assert (mongo_wire_reply_packet_get_nth_document (p, 1, &doc));
  bson_finish (doc);
  g_assert ((c = bson_find (doc, "int32")));
  g_assert (bson_cursor_get_int32 (c, &i));
  g_assert_cmpint (i, ==, 1984);
  bson_free (doc);
  g_free (c);

  mongo_wire_packet_free (p);

  mongo_disconnect (conn);
  PASS ();
}

void do_plan (int max)
{
  mongo_connection *conn;

  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  if (!conn)
    SKIP_ALL ("cannot connect to mongodb; host="
	      TEST_SERVER_IP);

  PLAN (1, max);
  mongo_disconnect (conn);
}

int
main (void)
{
  mongo_util_oid_init (0);
  do_plan (3);

  test_mongo_sync_cmd_insert ();
  test_mongo_sync_cmd_update ();
  test_mongo_sync_cmd_query ();

  return 0;
}
