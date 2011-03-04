#include "test.h"
#include "tap.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_cmd_get_more (void)
{
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  gint32 pos;
  gint64 cid = 9876543210;

  ok (mongo_wire_cmd_get_more (1, NULL, 1, cid) == NULL,
      "mongo_wire_cmd_get_more() fails with a NULL namespace");
  ok ((p = mongo_wire_cmd_get_more (1, "test.ns", 1, cid)) != NULL,
      "mongo_wire_cmd_get_more() works");

  /* Test basic header data */
  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size appears fine");

  cmp_ok (hdr.length, "==", sizeof (mongo_packet_header) + data_size,
	  "Packet header length is correct");
  cmp_ok (hdr.id, "==", 1, "Header ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Response ID is ok");

  /*
   * Test the request itself.
   */

  /* pos = zero + ns + NULL + ret */
  pos = sizeof (gint32) + strlen ("test.ns") + 1 + sizeof (gint32);
  cid = 0;
  memcpy (&cid, data + pos, sizeof (cid));
  cid = GINT64_FROM_LE (cid);

  ok (cid == 9876543210,
      "Included CID is correct");

  mongo_wire_packet_free (p);
}

RUN_TEST (7, mongo_wire_cmd_get_more);
