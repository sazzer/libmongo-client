#include "test.h"
#include "tap.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_reply_packet_get_data (void)
{
  mongo_packet *p;
  mongo_packet_header h;
  const guint8 *data;

  p = mongo_wire_packet_new ();
  memset (&h, 0, sizeof (mongo_packet_header));
  h.opcode = 0;
  h.length = sizeof (mongo_packet_header);
  mongo_wire_packet_set_header (p, &h);

  ok (mongo_wire_reply_packet_get_data (NULL, &data) == FALSE,
      "mongo_wire_reply_packet_get_data() fails with a NULL packet");
  ok (mongo_wire_reply_packet_get_data (p, NULL) == FALSE,
      "mongo_wire_reply_packet_get_data() fails with a NULL destination");
  ok (mongo_wire_reply_packet_get_data (p, &data) == FALSE,
      "mongo_wire_reply_packet_get_data() fails with a non-reply packet");

  h.opcode = 1;
  mongo_wire_packet_set_header (p, &h);

  ok (mongo_wire_reply_packet_get_data (p, &data) == FALSE,
      "mongo_wire_reply_packet_get_data() fails if the packet has "
      "no data");

  mongo_wire_packet_free (p);
}

RUN_TEST (4, mongo_wire_reply_packet_get_data);
