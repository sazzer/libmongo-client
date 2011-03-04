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

  mongo_wire_packet_set_header (p, &h);

  ok (mongo_wire_reply_packet_get_header (NULL, &rh) == FALSE,
      "mongo_wire_reply_packet_get_header() fails with a NULL packet");
  ok (mongo_wire_reply_packet_get_header (p, NULL) == FALSE,
      "mongo_wire_reply_packet_get_header() fails with a NULL header");

  ok (mongo_wire_reply_packet_get_header (p, &rh) == FALSE,
      "mongo_wire_reply_packet_get_header() fails if the packet has "
      "no reply header");

  mongo_wire_packet_free (p);
}

RUN_TEST (3, mongo_wire_reply_packet_get_header);
