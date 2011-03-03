#include "test.h"
#include "bson.h"
#include "mongo-wire.h"

#include <glib.h>

void
test_invalid_mongo_wire (void)
{
  bson *sel, *data, *u;
  mongo_packet *p;

  sel = bson_new ();
  bson_append_string (sel, "_id", "none", -1);
  bson_finish (sel);

  data = bson_new ();
  bson_append_string (data, "name", "nobody", -1);
  bson_append_int32 (data, "age", 120);
  bson_finish (data);

  u = bson_new ();

  TEST (mongo_wire_packet_free);
  mongo_wire_packet_free (NULL);
  PASS ();

  TEST (mongo_wire_packet_get_parts);
  g_assert_cmpint (mongo_wire_packet_get_header (NULL, NULL), ==, FALSE);
  g_assert_cmpint (mongo_wire_packet_get_data (NULL, NULL), ==, -1);
  PASS ();

  TEST (mongo_wire_cmd_update);
  g_assert (mongo_wire_cmd_update (1, NULL, 0, NULL, NULL) == NULL);
  g_assert (mongo_wire_cmd_update (1, NULL, 0, sel, NULL) == NULL);
  g_assert (mongo_wire_cmd_update (1, NULL, 0, NULL, data) == NULL);
  g_assert (mongo_wire_cmd_update (1, NULL, 0, sel, data) == NULL);
  g_assert (mongo_wire_cmd_update (1, "test.ns", 0, NULL, NULL) == NULL);
  g_assert (mongo_wire_cmd_update (1, "test.ns", 0, sel, u) == NULL);
  g_assert ((p = mongo_wire_cmd_update (1, "test.ns", 0, sel, data)));
  mongo_wire_packet_free (p);
  PASS ();

  TEST (mongo_wire_cmd_insert);
  g_assert (mongo_wire_cmd_insert (1, NULL, NULL, NULL) == NULL);
  g_assert (mongo_wire_cmd_insert (1, NULL, data, NULL) == NULL);
  g_assert (mongo_wire_cmd_insert (1, "test.ns", u, NULL) == NULL);
  g_assert ((p = mongo_wire_cmd_insert (1, "test.ns", data, NULL)));
  mongo_wire_packet_free (p);
  PASS ();

  TEST (mongo_wire_packet_new);
  g_assert ((p = mongo_wire_packet_new ()));
  mongo_wire_packet_free (p);
  PASS ();

  bson_free (u);
  bson_free (sel);
  bson_free (data);
}

int
main (void)
{
  PLAN (1, 18);

  test_invalid_mongo_wire ();

  return 0;
}
