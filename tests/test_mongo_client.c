#include "test.h"
#include "test-generator.h"

#include "mongo-client.h"
#include "mongo-wire.h"
#include "mongo-utils.h"
#include "bson.h"

#include <glib.h>

void
test_mongo_client (void)
{
  bson *sel, *upd;
  mongo_packet *p;
  mongo_connection *conn;

  TEST (mongo_client.connect);
  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  if (!conn)
    SKIP ();
  PASS ();

  sel = bson_new ();
  bson_append_null (sel, "_id");
  bson_finish (sel);

  upd = test_bson_generate_nested ();

  p = mongo_wire_cmd_update (1, TEST_SERVER_NS, 1, sel, upd);

  bson_free (sel);
  bson_free (upd);

  TEST (mongo_client.packet_send);
  g_assert (mongo_packet_send (conn, p));
  PASS ();

  TEST (mongo_client.disconnect);
  mongo_disconnect (conn);
  PASS ();
}

void
test_mongo_client_recv (void)
{
  bson *q;
  mongo_packet *p;
  mongo_connection *conn;
  mongo_packet_header h;
  const guint8 *data;
  guint pos;
  gint32 data_size;
  bson_cursor *c;

  TEST (mongo_client.recv);
  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  if (!conn)
    SKIP ();

  q = bson_new ();
  bson_append_string (q, "recv", "oh, yes!", -1);
  bson_finish (q);

  p = mongo_wire_cmd_insert (1, TEST_SERVER_NS, q, NULL);
  g_assert (mongo_packet_send (conn, p));
  mongo_wire_packet_free (p);

  p = mongo_wire_cmd_query (1, TEST_SERVER_NS, 0, 0, 1, q,
			    NULL);
  g_assert (mongo_wire_packet_get_header (p, &h));
  g_assert ((data_size = mongo_wire_packet_get_data (p, &data)) != -1);

  g_assert (mongo_packet_send (conn, p));
  mongo_wire_packet_free (p);
  bson_free (q);

  g_assert ((p = mongo_packet_recv (conn)) != NULL);

  g_assert_cmpint (mongo_wire_packet_get_header (p, &h), !=, -1);
  g_assert_cmpint (mongo_wire_packet_get_data (p, &data), !=, -1);

  pos = sizeof (gint32) /* resp. flags */ +
    sizeof (gint64) /* cursor ID */ +
    sizeof (gint32) /* starting from */ +
    sizeof (gint32); /* returned */

  g_assert ((q = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (q);

  g_assert ((c = bson_find (q, "recv")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_free (c);
  bson_free (q);

  mongo_wire_packet_free (p);
  mongo_disconnect (conn);

  PASS ();
}

void
test_mongo_client_recv_custom (void)
{
  bson *cmd;
  mongo_packet *p;
  mongo_connection *conn;
  mongo_packet_header h;
  const guint8 *data;
  guint pos;
  bson_cursor *c;

  gdouble ok;
  const gchar *nonce;

  TEST (mongo_client.recv.custom);
  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  if (!conn)
    SKIP ();

  cmd = bson_new ();
  bson_append_int32 (cmd, "getnonce", 1);
  bson_finish (cmd);

  p = mongo_wire_cmd_custom (1, TEST_SERVER_DB, cmd);
  g_assert (mongo_packet_send (conn, p));
  mongo_wire_packet_free (p);
  bson_free (cmd);

  g_assert ((p = mongo_packet_recv (conn)) != NULL);

  g_assert (mongo_wire_packet_get_header (p, &h));
  g_assert_cmpint (mongo_wire_packet_get_data (p, &data), !=, -1);

  pos = sizeof (gint32) /* resp. flags */ +
    sizeof (gint64) /* cursor ID */ +
    sizeof (gint32) /* starting from */ +
    sizeof (gint32); /* returned */

  g_assert ((cmd = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (cmd);

  g_assert ((c = bson_find (cmd, "ok")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOUBLE);
  g_assert (bson_cursor_get_double (c, &ok));
  g_assert (ok == 1);
  g_free (c);

  g_assert ((c = bson_find (cmd, "nonce")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_assert (bson_cursor_get_string (c, &nonce));
  printf ("   # nonce: %s\n", nonce);
  g_free (c);

  bson_free (cmd);
  mongo_wire_packet_free (p);
  mongo_disconnect (conn);

  PASS ();
}

void
test_mongo_client_reply_parse (void)
{
  bson *cmd;
  mongo_packet *p;
  mongo_connection *conn;
  bson_cursor *c;

  gdouble ok;
  const gchar *nonce;

  mongo_reply_packet_header rh;

  TEST (mongo_client.reply_parse);
  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  if (!conn)
    SKIP ();

  cmd = bson_new ();
  bson_append_int32 (cmd, "getnonce", 1);
  bson_finish (cmd);

  p = mongo_wire_cmd_custom (1, TEST_SERVER_DB, cmd);
  g_assert (mongo_packet_send (conn, p));
  mongo_wire_packet_free (p);
  bson_free (cmd);

  g_assert ((p = mongo_packet_recv (conn)) != NULL);

  g_assert (mongo_wire_reply_packet_get_header (p, &rh));

  g_assert_cmpint (rh.start, ==, 0);
  g_assert_cmpint (rh.returned, ==, 1);

  g_assert (mongo_wire_reply_packet_get_nth_document (p, 1, &cmd));
  bson_finish (cmd);

  g_assert ((c = bson_find (cmd, "ok")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOUBLE);
  g_assert (bson_cursor_get_double (c, &ok));
  g_assert (ok == 1);
  g_free (c);

  g_assert ((c = bson_find (cmd, "nonce")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_assert (bson_cursor_get_string (c, &nonce));
  printf ("   # nonce: %s\n", nonce);
  g_free (c);

  bson_free (cmd);

  g_assert (mongo_wire_reply_packet_get_nth_document (p, 2, &cmd) == FALSE);

  mongo_wire_packet_free (p);
  mongo_disconnect (conn);
  PASS ();
}

void
test_mongo_client_cursors (void)
{
  bson *doc;
  mongo_packet *p;
  mongo_connection *conn;
  bson_cursor *c;
  guint8 *oid;
  gint seq;

  mongo_reply_packet_header rh;
  const gchar *s;
  guint64 cid;
  gint32 i;

  TEST (mongo_client.cursors);
  conn = mongo_connect (TEST_SERVER_IP, TEST_SERVER_PORT);
  if (!conn)
    SKIP ();

  mongo_util_oid_init (0);

  /* Insert 30 documents... */
  for (seq = 1; seq <= 30; seq++)
    {
      doc = bson_new ();
      oid = mongo_util_oid_new (seq);
      bson_append_oid (doc, "_id", oid);
      g_free (oid);
      bson_append_string (doc, "str", "string1", -1);
      bson_append_int32 (doc, "seq", seq);
      bson_finish (doc);

      p = mongo_wire_cmd_insert (seq, TEST_SERVER_NS, doc, NULL);
      bson_free (doc);

      g_assert (mongo_packet_send (conn, p));
      mongo_wire_packet_free (p);
    }

  /* Construct a query to retrieve them... */
  doc = bson_new ();
  bson_append_string (doc, "str", "string1", -1);
  bson_finish (doc);

  p = mongo_wire_cmd_query (40, TEST_SERVER_NS,
			    MONGO_WIRE_FLAG_QUERY_NO_CURSOR_TIMEOUT,
			    0, 2, doc, NULL);
  bson_free (doc);
  g_assert (mongo_packet_send (conn, p));
  mongo_wire_packet_free (p);

  p = mongo_packet_recv (conn);

  /* Verify the headers */
  g_assert (mongo_wire_reply_packet_get_header (p, &rh));
  g_assert (rh.flags & MONGO_REPLY_FLAG_AWAITCAPABLE);
  g_assert (!(rh.flags & MONGO_REPLY_FLAG_NO_CURSOR));
  g_assert (!(rh.flags & MONGO_REPLY_FLAG_QUERY_FAIL));

  g_assert_cmpint (rh.cursor_id, !=, 0);
  g_assert_cmpint (rh.start, ==, 0);
  g_assert_cmpint (rh.returned, ==, 2);

  /* Verify the data */
  g_assert (mongo_wire_reply_packet_get_nth_document (p, 1, &doc));
  bson_finish (doc);
  g_assert ((c = bson_find (doc, "str")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_assert (bson_cursor_get_string (c, &s));
  g_assert_cmpstr (s, ==, "string1");
  g_free (c);
  bson_free (doc);

  cid = rh.cursor_id;
  mongo_wire_packet_free (p);

  /* Get more data... */
  p = mongo_wire_cmd_get_more (41, TEST_SERVER_NS, 4, cid);
  g_assert (mongo_packet_send (conn, p));
  mongo_wire_packet_free (p);

  p = mongo_packet_recv (conn);

  /* Verify second batch of headers */
  g_assert (mongo_wire_reply_packet_get_header (p, &rh));
  g_assert (rh.flags & MONGO_REPLY_FLAG_AWAITCAPABLE);
  g_assert (!(rh.flags & MONGO_REPLY_FLAG_NO_CURSOR));
  g_assert (!(rh.flags & MONGO_REPLY_FLAG_QUERY_FAIL));

  g_assert_cmpint (rh.cursor_id, !=, 0);
  g_assert_cmpint (rh.cursor_id, ==, cid);
  g_assert_cmpint (rh.start, ==, 2);
  g_assert_cmpint (rh.returned, ==, 4);

  /* Verify second batch of documents */
  g_assert (mongo_wire_reply_packet_get_nth_document (p, 4, &doc));
  bson_finish (doc);
  g_assert ((c = bson_find (doc, "seq")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_INT32);
  g_assert (bson_cursor_get_int32 (c, &i));
  g_assert_cmpint (i, ==, 6);
  g_free (c);
  bson_free (doc);
  mongo_wire_packet_free (p);

  /* Kill cursors */
  p = mongo_wire_cmd_kill_cursors (42, 1, cid);
  g_assert (mongo_packet_send (conn, p));
  mongo_wire_packet_free (p);

  mongo_disconnect (conn);
  PASS ();
}

int
main (void)
{
  test_mongo_client ();
  test_mongo_client_recv ();
  test_mongo_client_recv_custom ();
  test_mongo_client_reply_parse ();
  test_mongo_client_cursors ();

  return 0;
}
