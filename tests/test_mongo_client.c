#include "test.h"
#include "test-generator.h"

#include "mongo-client.h"
#include "mongo-wire.h"
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

int
main (void)
{
  test_mongo_client ();
  test_mongo_client_recv ();
  test_mongo_client_recv_custom ();
  test_mongo_client_reply_parse ();

  return 0;
}
