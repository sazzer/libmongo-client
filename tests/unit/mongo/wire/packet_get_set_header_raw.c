#include "tap.h"
#include "test.h"
#include "mongo-wire.h"

#include "libmongo-private.h"

#include <string.h>

void
test_mongo_wire_packet_get_set_header_raw (void)
{
  mongo_packet *p;
  mongo_packet_header ph1, ph2;

  p = mongo_wire_packet_new ();

  ok (mongo_wire_packet_get_header_raw (NULL, &ph2) == FALSE,
      "mongo_wire_packet_get_header_raw() should fail with a NULL packet");
  ok (mongo_wire_packet_get_header_raw (p, NULL) == FALSE,
      "mongo_wire_packet_get_header_raw() should fail with a NULL header");
  ok (mongo_wire_packet_set_header_raw (NULL, &ph1) == FALSE,
      "mongo_wire_packet_set_header_raw() should fail with a NULL packet");
  ok (mongo_wire_packet_set_header_raw (p, NULL) == FALSE,
      "mongo_wire_packet_set_header_raw() should fail with a NULL header");

  ok (mongo_wire_packet_get_header_raw (p, &ph2),
      "mongo_wire_packet_get_header_raw() works on a fresh packet");
  cmp_ok (ph2.length, "==", sizeof (mongo_packet_header),
	  "Initial packet length is the length of the header");

  ph1.length = sizeof (mongo_packet_header);
  ph1.id = 1;
  ph1.resp_to = 0;
  ph1.opcode = 1000;

  memset (&ph2, 0, sizeof (mongo_packet_header));

  ok (mongo_wire_packet_set_header_raw (p, &ph1),
      "mongo_wire_packet_set_header_raw() works");
  ok (mongo_wire_packet_get_header_raw (p, &ph2),
      "mongo_wire_packet_get_header_raw() works");

  cmp_ok (ph1.length, "==", ph2.length,
	  "Packet lengths match");
  cmp_ok (ph1.id, "==", ph2.id,
	  "Sequence IDs match");
  cmp_ok (ph1.resp_to, "==", ph2.resp_to,
	  "Response IDs match");
  cmp_ok (ph1.opcode, "==", ph2.opcode,
	  "OPCodes match");

  mongo_wire_packet_free (p);
}

RUN_TEST (12, mongo_wire_packet_get_set_header_raw);
