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

static void
test_mongo_wire_update (void)
{
  bson *sel, *upd;
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  bson_cursor *c;
  gint32 pos;

  TEST (mongo_wire.update);

  sel = bson_new ();
  g_assert (bson_append_null (sel, "_id"));
  bson_finish (sel);

  upd = test_bson_generate_nested ();

  g_assert ((p = mongo_wire_cmd_update (1, TEST_SERVER_NS, 0, sel, upd)));

  bson_free (sel);
  bson_free (upd);

  g_assert (mongo_wire_packet_get_header (p, &hdr));
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  g_assert_cmpint (hdr.length, ==,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (hdr.id, ==, 1);
  g_assert_cmpint (hdr.resp_to, ==, 0);

  /* pos = zero + collection_name + NULL + flags */
  pos = sizeof (gint32) + strlen (TEST_SERVER_NS) + 1 + sizeof (gint32);
  g_assert ((sel = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (sel);

  g_assert ((c = bson_find (sel, "_id")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_NULL);
  bson_cursor_free (c);
  bson_free (sel);

  pos += (gint32)data[pos];
  g_assert ((upd = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (upd);

  g_assert ((c = bson_find (upd, "user")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOCUMENT);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
  g_assert (!bson_cursor_next (c));

  bson_cursor_free (c);
  bson_free (upd);
  mongo_wire_packet_free (p);

  PASS ();
}

void
test_mongo_wire_insert ()
{
  bson *ins;
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  bson_cursor *c;
  gint32 pos;

  TEST (mongo_wire.insert);

  ins = test_bson_generate_nested ();
  g_assert ((p = mongo_wire_cmd_insert (1, TEST_SERVER_NS, ins, NULL)));
  bson_free (ins);

  g_assert (mongo_wire_packet_get_header (p, &hdr));
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  g_assert_cmpint (hdr.length, ==,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (hdr.id, ==, 1);
  g_assert_cmpint (hdr.resp_to, ==, 0);

  /* pos = zero + collection_name + NULL */
  pos = sizeof (gint32) + strlen (TEST_SERVER_NS) + 1;
  g_assert ((ins = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (ins);

  g_assert ((c = bson_find (ins, "user")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOCUMENT);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
  g_assert (!bson_cursor_next (c));

  bson_cursor_free (c);
  bson_free (ins);
  mongo_wire_packet_free (p);

  PASS ();
}

void
test_mongo_wire_query ()
{
  bson *q, *s;
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  bson_cursor *c;
  gint32 pos;

  TEST (mongo_wire.query);

  q = test_bson_generate_nested ();
  s = test_bson_generate_flat ();
  g_assert ((p = mongo_wire_cmd_query (1,TEST_SERVER_NS, 0, 0, 10, q, s)));
  bson_free (q);
  bson_free (s);

  g_assert (mongo_wire_packet_get_header (p, &hdr));
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  g_assert_cmpint (hdr.length, ==,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (hdr.id, ==, 1);
  g_assert_cmpint (hdr.resp_to, ==, 0);

  /* pos = zero + collection_name + NULL + skip + ret */
  pos = sizeof (gint32) + strlen (TEST_SERVER_NS) + 1 + sizeof (gint32) * 2;
  g_assert ((q = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (q);

  g_assert ((c = bson_find (q, "user")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOCUMENT);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
  g_assert (!bson_cursor_next (c));

  bson_cursor_free (c);
  bson_free (q);

  pos += (gint32)data[pos];
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
test_mongo_wire_setters (void)
{
  bson *n, *f;
  mongo_packet *p;

  const guint8 *rdata;
  gint32 data_size, pos;

  mongo_packet_header rhdr, whdr;
  guint8 *wdata;

  bson_cursor *c;

  TEST (mongo_wire.setters);

  n = test_bson_generate_nested ();
  f = test_bson_generate_flat ();

  g_assert ((p = mongo_wire_cmd_insert (1, TEST_SERVER_NS, n, NULL)));
  bson_free (n);

  g_assert (mongo_wire_packet_get_header (p, &rhdr));
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &rdata)), !=, -1);

  g_assert_cmpint (rhdr.length, ==,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (rhdr.id, ==, 1);
  g_assert_cmpint (rhdr.resp_to, ==, 0);

  memcpy (&whdr, &rhdr, sizeof (mongo_packet_header));
  g_assert ((wdata = g_try_new0 (guint8, data_size)) != NULL);

  whdr.id = 2;
  whdr.length = 0;

  g_assert (mongo_wire_packet_set_header (p, &whdr));
  g_assert (mongo_wire_packet_get_header (p, &rhdr));
  g_assert_cmpint (rhdr.length, ==, 0);
  g_assert_cmpint (rhdr.id, ==, 2);

  pos = sizeof (gint32) + strlen (TEST_SERVER_NS) + 1;
  memcpy (wdata, rdata, pos);
  memcpy (wdata + pos, bson_data (f), bson_size (f));

  g_assert (mongo_wire_packet_set_data (p, wdata, pos + bson_size (f)));
  bson_free (f);
  g_assert (mongo_wire_packet_get_header (p, &rhdr));

  g_assert_cmpint (rhdr.length, !=,
		   sizeof (mongo_packet_header) + data_size);
  g_assert_cmpint (rhdr.length, !=, 0);

  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &rdata)), !=, -1);
  g_assert ((f = bson_new_from_data (rdata + pos, (gint32)rdata[pos] - 1)));
  bson_finish (f);

  g_assert ((c = bson_find (f, "null")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_NULL);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BOOLEAN);

  bson_cursor_free (c);
  bson_free (f);
  g_free (wdata);
  mongo_wire_packet_free (p);

  PASS ();
}

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

  g_assert ((p = mongo_wire_cmd_custom (1, TEST_SERVER_DB, cmd)));
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
