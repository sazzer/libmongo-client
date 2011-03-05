#include "test.h"
#include "tap.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_cmd_kill_cursors (void)
{
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  gint32 pos, n = 0;
  gint64 c1 = 9876543210, c2 = 1234567890;

  ok (mongo_wire_cmd_kill_cursors (1, 0) == NULL,
      "mongo_wire_cmd_kill_cursors() should fail with zero cursors");
  ok (mongo_wire_cmd_kill_cursors (1, -1) == NULL,
      "mongo_wire_cmd_kill_cursors() should fail with negative amount of "
      "cursors");
  ok ((p = mongo_wire_cmd_kill_cursors (1, 2, c1, c2)) != NULL,
      "mongo_wire_cmd_kill_cursors() works");

  /* Verify the header */
  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size looks fine");
  cmp_ok (hdr.length, "==", sizeof (mongo_packet_header) + data_size,
	  "Packet header length is OK");
  cmp_ok (hdr.id, "==", 1, "Packet request ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Packet reply ID is ok");

  /*
   * Test the request contents
   */
  c1 = c2 = 0;
  /* pos = zero + n */
  pos = sizeof (gint32) + sizeof (n);

  memcpy (&n, data + sizeof (gint32), sizeof (gint32));
  memcpy (&c1, data + pos, sizeof (c1));
  memcpy (&c2, data + pos + sizeof (c1), sizeof (c2));

  n = GINT32_FROM_LE (n);
  c1 = GINT64_FROM_LE (c1);
  c2 = GINT64_FROM_LE (c2);

  ok (n == 2, "Number of cursors are OK");
  ok (c1 == 9876543210, "First cursor is OK");
  ok (c2 == 1234567890, "Second cursor is OK");

  mongo_wire_packet_free (p);
}

RUN_TEST (10, mongo_wire_cmd_kill_cursors);
