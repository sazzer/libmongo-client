#include "test.h"
#include "tap.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_cmd_custom (void)
{
  bson *cmd;
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  bson_cursor *c;
  gint32 pos;

  cmd = bson_new ();
  bson_append_int32 (cmd, "getnonce", 1);

  ok (mongo_wire_cmd_custom (1, "test", 0, NULL) == NULL,
      "mongo_wire_cmd_custom() fails with a NULL command");
  ok (mongo_wire_cmd_custom (1, "test", 0, cmd) == NULL,
      "mongo_wire_cmd_custom() fails with an unfinished command");
  bson_finish (cmd);
  ok (mongo_wire_cmd_custom (1, NULL, 0, cmd) == NULL,
      "mongo_wire_cmd_custom() fails with a NULL db");

  ok ((p = mongo_wire_cmd_custom (1, "test", 0, cmd)) != NULL,
      "mongo_wire_cmd_custom() works");
  bson_free (cmd);

  /* Verify the header */
  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size looks fine");
  cmp_ok (hdr.length, "==", sizeof (mongo_packet_header) + data_size,
	  "Packet header length is OK");
  cmp_ok (hdr.id, "==", 1, "Packet request ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Packet reply ID is ok");

  /*
   * Test the created request
   */

  /* pos = zero + collection_name + NULL + skip + ret */
  pos = sizeof (gint32) + strlen ("test.$cmd") + 1 + sizeof (gint32) * 2;
  ok ((cmd = bson_new_from_data (data + pos,
				 _DOC_SIZE (data, pos) - 1)) != NULL,
      "Packet contains a BSON document");
  bson_finish (cmd);

  ok ((c = bson_find (cmd, "getnonce")) != NULL,
      "BSON object contains a 'getnonce' key");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT32,
	  "'getnonce' key has the correct type");
  ok (bson_cursor_next (c) == FALSE,
      "'getnonce' key is the last in the object");

  bson_cursor_free (c);
  bson_free (cmd);
  mongo_wire_packet_free (p);
}

RUN_TEST (12, mongo_wire_cmd_custom);
