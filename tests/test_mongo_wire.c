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

  const guint8 *hdr, *data;
  gint32 hdr_size, data_size;

  bson_cursor *c;
  gint32 pos;

  TEST (mongo_wire.update);

  sel = bson_new ();
  g_assert (bson_append_null (sel, "_id"));
  bson_finish (sel);

  upd = test_bson_generate_nested ();

  g_assert ((p = mongo_wire_cmd_update (1, "test.libmongo", 0, sel, upd)));

  bson_free (sel);
  bson_free (upd);

  g_assert_cmpint ((hdr_size = mongo_wire_packet_get_header (p, &hdr)), !=, -1);
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  /* pos = zero + collection_name + NULL + flags */
  pos = sizeof (gint32) + strlen ("test.libmongo") + 1 + sizeof (gint32);
  g_assert ((sel = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (sel);

  g_assert ((c = bson_find (sel, "_id")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_NULL);
  g_free (c);
  bson_free (sel);

  pos += (gint32)data[pos];
  g_assert ((upd = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (upd);

  g_assert ((c = bson_find (upd, "user")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOCUMENT);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
  g_assert (!bson_cursor_next (c));

  g_free (c);
  bson_free (upd);

  PASS ();
}

void
test_mongo_wire_insert ()
{
  bson *ins;
  mongo_packet *p;

  const guint8 *hdr, *data;
  gint32 hdr_size, data_size;

  bson_cursor *c;
  gint32 pos;

  TEST (mongo_wire.insert);

  ins = test_bson_generate_nested ();
  g_assert ((p = mongo_wire_cmd_insert (1, "test.libmongo", ins)));
  bson_free (ins);

  g_assert_cmpint ((hdr_size = mongo_wire_packet_get_header (p, &hdr)), !=, -1);
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  /* pos = zero + collection_name + NULL */
  pos = sizeof (gint32) + strlen ("test.libmongo") + 1;
  g_assert ((ins = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (ins);

  g_assert ((c = bson_find (ins, "user")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOCUMENT);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
  g_assert (!bson_cursor_next (c));

  g_free (c);
  bson_free (ins);

  PASS ();
}

void
test_mongo_wire_query ()
{
  bson *q, *s;
  mongo_packet *p;

  const guint8 *hdr, *data;
  gint32 hdr_size, data_size;

  bson_cursor *c;
  gint32 pos;

  TEST (mongo_wire.query);

  q = test_bson_generate_nested ();
  s = test_bson_generate_flat ();
  g_assert ((p = mongo_wire_cmd_query (1, "test.libmongo", 0, 0, 10, q, s)));
  bson_free (q);
  bson_free (s);

  g_assert_cmpint ((hdr_size = mongo_wire_packet_get_header (p, &hdr)), !=, -1);
  g_assert_cmpint ((data_size = mongo_wire_packet_get_data (p, &data)), !=, -1);

  /* pos = zero + collection_name + NULL + skip + ret */
  pos = sizeof (gint32) + strlen ("test.libmongo") + 1 + sizeof (gint32) * 2;
  g_assert ((q = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (q);

  g_assert ((c = bson_find (q, "user")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOCUMENT);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
  g_assert (!bson_cursor_next (c));

  g_free (c);
  bson_free (q);

  pos += (gint32)data[pos];
  g_assert ((s = bson_new_from_data (data + pos, (gint32)data[pos] - 1)));
  bson_finish (s);

  g_assert ((c = bson_find (s, "null")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_NULL);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BOOLEAN);

  g_free (c);
  bson_free (s);

  PASS ();
}


int
main (void)
{
  test_mongo_wire_update ();
  test_mongo_wire_insert ();
  test_mongo_wire_query ();

  return 0;
}
