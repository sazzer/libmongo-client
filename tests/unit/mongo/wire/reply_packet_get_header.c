#include "test.h"
#include "tap.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_reply_packet_get_header (void)
{
  mongo_packet *p;
  mongo_packet_header h;
  mongo_reply_packet_header rh;

  p = mongo_wire_packet_new ();
  memset (&h, 0, sizeof (mongo_packet_header));
  h.opcode = 1;
  h.length = sizeof (mongo_packet_header);

  mongo_wire_packet_set_header (p, &h);

  ok (mongo_wire_reply_packet_get_header (NULL, &rh) == FALSE,
      "mongo_wire_reply_packet_get_header() fails with a NULL packet");
  ok (mongo_wire_reply_packet_get_header (p, NULL) == FALSE,
      "mongo_wire_reply_packet_get_header() fails with a NULL header");

  ok (mongo_wire_reply_packet_get_header (p, &rh) == FALSE,
      "mongo_wire_reply_packet_get_header() fails if the packet has "
      "no reply header");

  h.opcode = 2;
  mongo_wire_packet_set_header (p, &h);
  ok (mongo_wire_reply_packet_get_header (p, &rh) == FALSE,
      "mongo_wire_reply_packet_get_header() fails if the packet is "
      "not a reply packet");

  mongo_wire_packet_free (p);

  p = test_mongo_wire_generate_reply (TRUE, 0, FALSE);

  ok (mongo_wire_reply_packet_get_header (p, &rh),
      "mongo_wire_reply_packet_get_header() works");
  cmp_ok (rh.flags, "==", 0,
	  "Reply flags are correct");
  ok (rh.cursor_id == (gint64)12345,
      "Cursor ID is correct");
  cmp_ok (rh.start, "==", 0,
	  "Reply start document is OK");
  cmp_ok (rh.returned, "==", 0,
	  "Number of documents returned is OK");

  mongo_wire_packet_free (p);
}

RUN_TEST (9, mongo_wire_reply_packet_get_header);
