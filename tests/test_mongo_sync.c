#include "test-network.h"
#include "test-generator.h"

#include "mongo.h"

#include <errno.h>
#include <string.h>
#include <glib.h>

void
test_mongo_sync_cmd_insert (void)
{
  bson *doc;
  mongo_sync_connection *conn;

  TEST (mongo_sync.cmd_insert);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  doc = test_bson_generate_nested ();

  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc, NULL));
  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc, NULL));

  bson_free (doc);

  g_assert_cmpint (mongo_connection_get_requestid ((mongo_connection *)conn),
		   >=, 2);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_update (void)
{
  bson *sel, *upd;
  mongo_sync_connection *conn;
  guint8 *oid;

  TEST (mongo_sync.cmd_update);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
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

  g_assert_cmpint (mongo_connection_get_requestid ((mongo_connection *)conn), >=, 2);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_query (void)
{
  bson *doc, *sel;
  mongo_sync_connection *conn;
  mongo_packet *p;

  bson_cursor *c;
  mongo_reply_packet_header rh;
  gint32 i;

  TEST (mongo_sync.cmd_query);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  doc = test_bson_generate_flat ();
  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc, doc, NULL));
  bson_free (doc);
  g_assert_cmpint (mongo_connection_get_requestid ((mongo_connection *)conn), >=, 1);

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
  bson_cursor_free (c);

  mongo_wire_packet_free (p);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_get_more (void)
{
  bson *doc, *sel;
  mongo_sync_connection *conn;
  mongo_packet *p;

  bson_cursor *c;
  mongo_reply_packet_header rh;
  gint32 i;
  gint64 cid;

  TEST (mongo_sync.cmd_get_more);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  doc = bson_new ();
  for (i = 1; i <= 30; i++)
    {
      bson_reset (doc);
      bson_append_int32 (doc, "int32", 1984);
      bson_append_int32 (doc, "seq", i);
      bson_append_boolean (doc, "get_more_test", TRUE);
      bson_finish (doc);
      g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc, NULL));
    }
  bson_free (doc);
  g_assert_cmpint (mongo_connection_get_requestid ((mongo_connection *)conn), >=, 30);

  sel = bson_new ();
  bson_append_boolean (sel, "get_more_test", TRUE);
  bson_finish (sel);
  g_assert ((p = mongo_sync_cmd_query (conn, TEST_SERVER_NS,
				       MONGO_WIRE_FLAG_QUERY_NO_CURSOR_TIMEOUT,
				       0, 2, sel, NULL)) != NULL);
  bson_free (sel);
  g_assert (mongo_wire_reply_packet_get_header (p, &rh));
  g_assert_cmpint (rh.cursor_id, !=, 0);
  g_assert_cmpint (rh.start, ==, 0);
  g_assert_cmpint (rh.returned, ==, 2);

  cid = rh.cursor_id;

  mongo_wire_packet_free (p);

  g_assert ((p = mongo_sync_cmd_get_more
	     (conn, TEST_SERVER_NS, 4, cid)) != NULL);

  g_assert (mongo_wire_reply_packet_get_header (p, &rh));
  g_assert_cmpint (rh.cursor_id, ==, cid);
  g_assert_cmpint (rh.start, ==, 2);
  g_assert_cmpint (rh.returned, ==, 4);

  g_assert (mongo_wire_reply_packet_get_nth_document (p, 1, &doc));
  bson_finish (doc);

  g_assert ((c = bson_find (doc, "int32")));
  g_assert (bson_cursor_get_int32 (c, &i));
  g_assert_cmpint (i, ==, 1984);
  bson_cursor_free (c);

  g_assert ((c = bson_find (doc, "seq")));
  g_assert (bson_cursor_get_int32 (c, &i));
  g_assert_cmpint (i, ==, 3);
  bson_cursor_free (c);

  bson_free (doc);

  mongo_wire_packet_free (p);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_delete (void)
{
  bson *b;
  mongo_sync_connection *conn;
  gint32 i;
  mongo_packet *p;

  bson_cursor *c;

  TEST (mongo_sync.cmd_delete);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  b = bson_new ();
  for (i = 1; i < 10; i++)
    {
      bson_reset (b);
      bson_append_int32 (b, "sync_delete_seq", i);
      bson_append_boolean (b, "sync_delete_flag", TRUE);
      bson_finish (b);
      g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, b, NULL));
    }
  bson_free (b);
  g_assert_cmpint (mongo_connection_get_requestid ((mongo_connection *)conn), >=, 9);

  b = bson_new ();
  bson_append_boolean (b, "sync_delete_flag", TRUE);
  bson_finish (b);
  g_assert (mongo_sync_cmd_delete (conn, TEST_SERVER_NS,
				   MONGO_WIRE_FLAG_DELETE_SINGLE, b));

  g_assert ((p = mongo_sync_cmd_query (conn, TEST_SERVER_NS, 0, 0, 2, b,
				       NULL)) != NULL);
  bson_free (b);
  g_assert (mongo_wire_reply_packet_get_nth_document (p, 1, &b));
  bson_finish (b);
  g_assert ((c = bson_find (b, "sync_delete_seq")));
  g_assert (bson_cursor_get_int32 (c, &i));
  bson_cursor_free (c);
  bson_free (b);
  mongo_wire_packet_free (p);

  mongo_sync_disconnect (conn);
  PASS();
}

void
test_mongo_sync_cmd_kill_cursor (void)
{
  bson *doc, *sel;
  mongo_sync_connection *conn;
  mongo_packet *p;

  mongo_reply_packet_header rh;
  gint32 i;
  gint64 cid;

  TEST (mongo_sync.cmd_get_more);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  doc = bson_new ();
  for (i = 1; i <= 30; i++)
    {
      bson_reset (doc);
      bson_append_int32 (doc, "int32", 1984);
      bson_append_int32 (doc, "seq", i);
      bson_append_boolean (doc, "kill_cursor_test", TRUE);
      bson_finish (doc);
      g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc, NULL));
    }
  bson_free (doc);
  g_assert_cmpint (mongo_connection_get_requestid ((mongo_connection *)conn), >=, 30);

  sel = bson_new ();
  bson_append_boolean (sel, "kill_cursor_test", TRUE);
  bson_finish (sel);
  g_assert ((p = mongo_sync_cmd_query (conn, TEST_SERVER_NS,
				       MONGO_WIRE_FLAG_QUERY_NO_CURSOR_TIMEOUT,
				       0, 2, sel, NULL)) != NULL);
  bson_free (sel);
  g_assert (mongo_wire_reply_packet_get_header (p, &rh));
  g_assert_cmpint (rh.cursor_id, !=, 0);
  g_assert_cmpint (rh.start, ==, 0);
  g_assert_cmpint (rh.returned, ==, 2);

  cid = rh.cursor_id;

  mongo_wire_packet_free (p);

  g_assert (mongo_sync_cmd_kill_cursors (conn, 1, cid));

  g_assert ((p = mongo_sync_cmd_get_more
	     (conn, TEST_SERVER_NS, 4, cid)) == NULL);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_custom (void)
{
  bson *b;
  mongo_packet *p;
  mongo_sync_connection *conn;

  bson_cursor *c;
  gdouble ok;
  const gchar *nonce;

  TEST (mongo_sync.cmd_custom);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, TRUE);
  g_assert (conn);

  b = bson_new ();
  bson_append_int32 (b, "getnonce", 1);
  bson_finish (b);

  g_assert ((p = mongo_sync_cmd_custom (conn, TEST_SERVER_DB, b)) != NULL);
  bson_free (b);

  g_assert (mongo_wire_reply_packet_get_nth_document (p, 1, &b));
  bson_finish (b);

  g_assert ((c = bson_find (b, "ok")));
  g_assert (bson_cursor_get_double (c, &ok));
  g_assert (ok == 1);
  bson_cursor_free (c);

  g_assert ((c = bson_find (b, "nonce")));
  g_assert (bson_cursor_get_string (c, &nonce));
  printf (" # nonce: %s\n", nonce);
  bson_cursor_free (c);

  bson_free (b);
  mongo_wire_packet_free (p);
  mongo_sync_disconnect (conn);

  PASS ();
}

void
test_mongo_sync_cmd_count (void)
{
  gdouble cnt1, cnt2;
  mongo_sync_connection *conn;
  bson *b;

  TEST (mongo_sync.cmd_count);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, TRUE);
  g_assert (conn);

  cnt1 = mongo_sync_cmd_count (conn, TEST_SERVER_DB, TEST_SERVER_COLLECTION,
			      NULL);
  g_assert_cmpint (cnt1, >, 0);

  b = bson_new ();
  bson_append_int32 (b, "int32", 1984);
  bson_finish (b);
  cnt2 = mongo_sync_cmd_count (conn, TEST_SERVER_DB, TEST_SERVER_COLLECTION,
			       b);
  bson_free (b);
  g_assert_cmpint (cnt2, >, 0);
  g_assert_cmpint (cnt1, >, cnt2);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_get_last_error (void)
{
  mongo_sync_connection *conn;
  bson *b;
  gchar *err;
  mongo_packet *p;

  TEST (mongo_sync.cmd_get_last_error);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  b = bson_new ();
  bson_append_int32 (b, "int32", 1984);
  bson_finish (b);
  mongo_sync_cmd_insert (conn, TEST_SERVER_NS, b, NULL);
  bson_free (b);

  g_assert (mongo_sync_cmd_get_last_error (conn, TEST_SERVER_DB, &err));
  g_assert (err == NULL);

  b = bson_new ();
  bson_append_int32 (b, "forceerror", 1);
  bson_finish (b);
  g_assert ((p = mongo_sync_cmd_custom (conn, TEST_SERVER_DB, b)) == NULL);
  mongo_wire_packet_free (p);
  bson_free (b);

  g_assert (mongo_sync_cmd_get_last_error (conn, TEST_SERVER_DB, &err));
  g_assert (err != NULL);
  printf (" # err: %s\n", err);
  g_free (err);
  g_assert (mongo_sync_cmd_get_last_error (conn, TEST_SERVER_DB, &err));
  g_assert (err != NULL);
  g_free (err);

  g_assert (mongo_sync_cmd_reset_error (conn, TEST_SERVER_DB));
  g_assert (mongo_sync_cmd_get_last_error (conn, TEST_SERVER_DB, &err));
  g_assert (err == NULL);

  mongo_sync_disconnect (conn);
  PASS();

  TEST (mongo_sync.cmd_get_last_error.from_query);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  b = bson_new ();
  bson_append_int32 (b, "foobar", 1);
  bson_finish (b);

  g_assert (mongo_sync_cmd_custom (conn, TEST_SERVER_DB, b) == NULL);
  bson_free (b);

  g_assert (mongo_sync_cmd_get_last_error (conn, TEST_SERVER_DB, &err));
  g_assert (err != NULL);
  printf (" # err: %s\n", err);
  g_free (err);

  mongo_sync_disconnect (conn);
  PASS();
}

void
test_mongo_sync_connect (void)
{
  mongo_sync_connection *conn, *o;

  TEST (mongo_sync.connect.ipv4);
  g_assert ((conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, TRUE)) != NULL);
  g_assert (mongo_sync_cmd_reset_error (conn, TEST_SERVER_DB));
  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_sync.connect.to_master.self);
  g_assert ((conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, TRUE)) != NULL);
  o = conn;
  g_assert ((conn = mongo_sync_reconnect (conn, TRUE)) != NULL);
  g_assert (o == conn);
  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_sync.connect.to_master.via_secondary);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  if (conn)
    {
      o = conn;
      g_assert ((conn = mongo_sync_reconnect (conn, TRUE)) != NULL);
      g_assert (o == conn);
      g_assert (mongo_sync_cmd_is_master (conn) == TRUE);
      g_assert (mongo_sync_cmd_reset_error (conn, TEST_SERVER_DB));
      mongo_sync_disconnect (conn);
      PASS ();
    }
  else
    {
      printf ("# %s\n", strerror (errno));
      SKIP ("Connecting via secondary failed, but it's optional.");
    }

  TEST (mongo_sync.connect.by_host);
  if (!TEST_SERVER_HOST)
    SKIP ("TEST_SERVER_HOST variable not set")
  else
    {
      g_assert ((conn = mongo_sync_connect (TEST_SERVER_HOST, TEST_SERVER_PORT, TRUE)) != NULL);
      g_assert (mongo_sync_cmd_reset_error (conn, TEST_SERVER_DB));
      mongo_sync_disconnect (conn);
      PASS ();
    }

  TEST (mongo_sync.connect.ipv6);
  if (!TEST_SERVER_IPV6)
    SKIP ("TEST_SERVER_IPV6 variable not set")
  else
    {
      conn = mongo_sync_connect (TEST_SERVER_IPV6, TEST_SERVER_PORT, TRUE);
      if (conn)
	{
	  g_assert (mongo_sync_cmd_reset_error (conn, TEST_SERVER_DB));
	  mongo_sync_disconnect (conn);
	  PASS ();
	}
      else
	{
	  printf ("# %s\n", strerror (errno));
	  SKIP ("IPv6 connection failed, but it's optional.");
	}
    }
}

void
test_mongo_sync_cmd_drop (void)
{
  mongo_sync_connection *conn;

  TEST (mongo_sync.cmd_drop);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_drop (conn, TEST_SERVER_DB,
				 TEST_SERVER_COLLECTION));
  g_assert (!mongo_sync_cmd_drop (conn, TEST_SERVER_DB,
				  TEST_SERVER_COLLECTION));

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_set_slaveok (void)
{
  mongo_sync_connection *conn;

  TEST (mongo_sync.set_slaveok);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_conn_get_slaveok (conn) == FALSE);
  mongo_sync_conn_set_slaveok (conn, TRUE);
  g_assert (mongo_sync_conn_get_slaveok (conn) == TRUE);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_ping (void)
{
  mongo_sync_connection *conn;

  TEST (mongo_sync.cmd.ping);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_ping (conn));

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_sync_cmd_authenticate (void)
{
  mongo_sync_connection *conn;

  TEST (mongo_sync.cmd.authenticate.setup);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);
  PASS ();

  TEST (mongo_sync.cmd.authenticate.user_add);
  if (!mongo_sync_cmd_user_add (conn, TEST_SERVER_DB, "test", "s3kr1+"))
    {
      if (errno == ENOTSUP)
	SKIP ("Authentication support not compiled in.")
      else
	FAIL ();
    }
  else
    PASS ();

  TEST (mongo_sync.cmd.authenticate.authenticate);
  if (!mongo_sync_cmd_authenticate (conn, TEST_SERVER_DB, "test", "s3kr1+"))
    {
      if (errno == ENOTSUP)
	SKIP ("Authentication support not compiled in.")
      else
	FAIL ();
    }
  else
    PASS ();

  TEST (mongo_sync.cmd.authenticate.user_remove);
  if (!mongo_sync_cmd_user_remove (conn, TEST_SERVER_DB, "test"))
    {
      if (errno == ENOTSUP)
	SKIP ("Authentication support not compiled in.")
      else
	FAIL ();
    }
  else
    PASS ();

  mongo_sync_disconnect (conn);
}

void
do_plan (int max)
{
  mongo_sync_connection *conn;

  if (!test_getenv_server ())
    SKIP_ALL ("TEST_SERVER variable not set");
  test_getenv_server_extra ();
  test_getenv_secondary ();

  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  if (!conn)
    SKIP_ALL ("cannot connect to mongodb");

  PLAN (1, max);
  mongo_sync_disconnect (conn);
}

int
main (void)
{
  mongo_util_oid_init (0);
  do_plan (22);

  test_mongo_sync_set_slaveok ();
  test_mongo_sync_cmd_insert ();
  test_mongo_sync_cmd_update ();
  test_mongo_sync_cmd_query ();
  test_mongo_sync_cmd_get_more ();
  test_mongo_sync_cmd_delete ();
  test_mongo_sync_cmd_kill_cursor ();
  test_mongo_sync_cmd_custom ();

  test_mongo_sync_cmd_count ();
  test_mongo_sync_cmd_get_last_error ();
  test_mongo_sync_cmd_drop ();
  test_mongo_sync_cmd_ping ();

  test_mongo_sync_cmd_authenticate ();

  test_mongo_sync_connect ();

  test_env_free ();

  return 0;
}
