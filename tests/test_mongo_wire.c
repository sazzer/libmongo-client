#include <glib.h>

#include "test.h"
#include "bson.h"
#include "mongo-wire.h"
#include "test-generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

void
test_mongo_wire_get_more (void)
{
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  gint32 pos;
  gint64 cid = 9876543210;

  TEST (mongo_wire.get_more);

  g_assert ((p = mongo_wire_cmd_get_more (1, TEST_SERVER_NS, 1, cid)));

  g_assert (mongo_wire_packet_get_header (p, &hdr));
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  g_assert_cmpint (hdr.length, ==,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (hdr.id, ==, 1);
  g_assert_cmpint (hdr.resp_to, ==, 0);

  /* pos = zero + ns + NULL + ret */
  pos = sizeof (gint32) + strlen (TEST_SERVER_NS) + 1 + sizeof (gint32);
  cid = 0;
  memcpy (&cid, data + pos, sizeof (cid));
  cid = GINT64_FROM_LE (cid);

  g_assert_cmpint (cid, ==, 9876543210);

  mongo_wire_packet_free (p);

  PASS();
}

void
test_mongo_wire_delete (void)
{
  mongo_packet *p;
  bson *s;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  gint32 pos;
  bson_cursor *c;

  TEST (mongo_wire.delete);

  s = test_bson_generate_flat ();
  g_assert ((p = mongo_wire_cmd_delete (1, TEST_SERVER_NS, 0, s)));
  bson_free (s);

  g_assert (mongo_wire_packet_get_header (p, &hdr));
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  g_assert_cmpint (hdr.length, ==,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (hdr.id, ==, 1);
  g_assert_cmpint (hdr.resp_to, ==, 0);

  /* pos = zero + ns + NULL + flags */
  pos = sizeof (gint32) + strlen (TEST_SERVER_NS) + 1 + sizeof (gint32);
  g_assert ((s = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (s);

  g_assert ((c = bson_find (s, "null")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_NULL);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BOOLEAN);

  bson_cursor_free (c);
  bson_free (s);

  mongo_wire_packet_free (p);

  PASS ();
}

void
test_mongo_wire_kill_cursors (void)
{
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  gint32 pos, n = 0;
  gint64 c1 = 9876543210, c2 = 1234567890;

  TEST (mongo_wire.kill_cursors);

  g_assert ((p = mongo_wire_cmd_kill_cursors (1, 2, c1, c2)));

  g_assert (mongo_wire_packet_get_header (p, &hdr));
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  g_assert_cmpint (hdr.length, ==,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (hdr.id, ==, 1);
  g_assert_cmpint (hdr.resp_to, ==, 0);

  c1 = c2 = 0;

  /* pos = zero + n */
  pos = sizeof (gint32) + sizeof (n);

  memcpy (&n, data + sizeof (gint32), sizeof (gint32));
  memcpy (&c1, data + pos, sizeof (c1));
  memcpy (&c2, data + pos + sizeof (c1), sizeof (c2));

  n = GINT32_FROM_LE (n);
  c1 = GINT64_FROM_LE (c1);
  c2 = GINT64_FROM_LE (c2);

  g_assert_cmpint (n, ==, 2);
  g_assert_cmpint (c1, ==, 9876543210);
  g_assert_cmpint (c2, ==, 1234567890);

  mongo_wire_packet_free (p);

  PASS ();
}

void
test_mongo_wire_custom ()
{
  bson *cmd;
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  bson_cursor *c;
  gint32 pos;

  TEST (mongo_wire.custom);

  cmd = bson_new ();
  bson_append_int32 (cmd, "getnonce", 1);
  bson_finish (cmd);

  g_assert ((p = mongo_wire_cmd_custom (1, TEST_SERVER_DB, 0, cmd)));
  bson_free (cmd);

  g_assert (mongo_wire_packet_get_header (p, &hdr));
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  g_assert_cmpint (hdr.length, ==,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (hdr.id, ==, 1);
  g_assert_cmpint (hdr.resp_to, ==, 0);

  /* pos = zero + collection_name + NULL + skip + ret */
  pos = sizeof (gint32) + strlen (TEST_SERVER_DB ".$cmd") + 1 + sizeof (gint32) * 2;
  g_assert ((cmd = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (cmd);

  g_assert ((c = bson_find (cmd, "getnonce")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_INT32);
  g_assert (!bson_cursor_next (c));

  bson_cursor_free (c);
  bson_free (cmd);
  mongo_wire_packet_free (p);

  PASS ();
}

int
main (void)
{
  PLAN (1, 8);

  test_mongo_wire_update ();
  test_mongo_wire_insert ();
  test_mongo_wire_query ();
  test_mongo_wire_setters ();
  test_mongo_wire_get_more ();
  test_mongo_wire_delete ();
  test_mongo_wire_kill_cursors ();

  test_mongo_wire_custom ();

  return 0;
}
