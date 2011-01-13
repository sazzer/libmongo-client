#include "test.h"
#include "test-generator.h"

#include "mongo-client.h"
#include "mongo_wire.h"
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

int
main (void)
{
  test_mongo_client ();
  return 0;
}
