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
  int fd;

  TEST (mongo_client.connect);
  fd = mongo_connect ("127.0.0.1", 27017);
  if (fd < 0)
    SKIP ();
  PASS ();

  sel = bson_new ();
  bson_append_null (sel, "_id");
  bson_finish (sel);

  upd = test_bson_generate_nested ();

  p = mongo_wire_cmd_update (1, "test.libmongo", 1, sel, upd);

  bson_free (sel);
  bson_free (upd);

  TEST (mongo_client.packet_send);
  g_assert (mongo_packet_send (fd, p));
  PASS ();

  TEST (mongo_client.disconnect);
  mongo_disconnect (fd);
  PASS ();
}

void
test_mongo_client_recv (void)
{
  bson *q;
  mongo_packet *p;
  int fd;
  const mongo_packet_header *h;
  const guint8 *data;
  guint pos;

  TEST (mongo_client.recv);
  fd = mongo_connect ("127.0.0.1", 27017);
  if (fd < 0)
    SKIP ();

  q = bson_new ();
  bson_append_string (q, "recv", "oh, yes!", -1);
  bson_finish (q);

  dump_data (bson_data (q), bson_size (q));

  p = mongo_wire_cmd_insert (1, "test.libmongo", q);
  g_assert (mongo_packet_send (fd, p));
  mongo_wire_packet_free (p);

  p = mongo_wire_cmd_query (1, "test.libmongo", 0, 0, 10, q,
			    NULL);
  g_assert (mongo_packet_send (fd, p));
  mongo_wire_packet_free (p);
  bson_free (q);

  g_assert ((p = mongo_packet_recv (fd)) != NULL);

  g_assert_cmpint (mongo_wire_packet_get_header (p, &h), !=, -1);
  g_assert_cmpint (mongo_wire_packet_get_data (p, &data), !=, -1);

  PASS ();
}

int
main (void)
{
  test_mongo_client ();
  test_mongo_client_recv ();

  return 0;
}
