#include "test.h"
#include "tap.h"
#include "mongo-wire.h"
#include "bson.h"

#include <string.h>

void
test_mongo_wire_reply_packet_get_nth_document (void)
{
  mongo_packet *p;
  bson *doc;
  mongo_packet_header h;

  p = mongo_wire_packet_new ();
  memset (&h, 0, sizeof (mongo_packet_header));
  h.opcode = 2;
  h.length = sizeof (mongo_packet_header);
  mongo_wire_packet_set_header (p, &h);

  ok (mongo_wire_reply_packet_get_nth_document (NULL, 1, &doc) == FALSE,
      "mongo_wire_reply_packet_get_nth_document() fails with a NULL packet");
  ok (mongo_wire_reply_packet_get_nth_document (p, 0, &doc) == FALSE,
      "mongo_wire_reply_packet_get_nth_document() fails with n = 0");
  ok (mongo_wire_reply_packet_get_nth_document (p, -42, &doc) == FALSE,
      "mongo_wire_reply_packet_get_nth_document() fails with n < 0");
  ok (mongo_wire_reply_packet_get_nth_document (p, 1, NULL) == FALSE,
      "mongo_wire_reply_packet_get_nth_document() fails with a NULL "
      "destination");

  ok (mongo_wire_reply_packet_get_nth_document (p, 1, &doc) == FALSE,
      "mongo_wire_reply_packet_get_nth_document() fails with a "
      "non-reply packet");

  h.opcode = 1;
  mongo_wire_packet_set_header (p, &h);

  ok (mongo_wire_reply_packet_get_nth_document (p, 1, &doc) == FALSE,
      "mongo_wire_reply_packet_get_nth_document() fails with an "
      "incomplete reply packet");

  mongo_wire_packet_free (p);
}

RUN_TEST (6, mongo_wire_reply_packet_get_nth_document);
