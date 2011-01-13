#include <glib.h>

#include "bson.h"
#include "mongo_wire.h"
#include "test-generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static void
test_mongo_wire_update (void)
{
  bson *sel, *upd;
  mongo_packet *p;

  const guint8 *hdr, *data;
  gint32 hdr_size, data_size;

  sel = bson_new ();
  g_assert (bson_append_null (sel, "_id"));
  bson_finish (sel);

  upd = test_bson_generate_nested ();

  g_assert ((p = mongo_wire_cmd_update (1, "test.libmongo", 0, sel, upd)));

  bson_free (sel);
  bson_free (upd);

  g_assert ((hdr_size = mongo_wire_packet_get_header (p, &hdr)));
  g_assert ((data_size = mongo_wire_packet_get_data (p, &data)));

  /*
   * FIXME: Right here, we should load up the bson from `data' and
   * parse it to verify that everything's in order.
   */
  /*
    sel = (bson *)(data + sizeof (gint32) + strlen ("test.libmongo") + 1 +
		 sizeof (gint32));
  */
}

int
main (void)
{
  test_mongo_wire_update ();

  return 0;
}
