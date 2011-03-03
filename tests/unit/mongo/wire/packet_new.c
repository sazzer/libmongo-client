#include "tap.h"
#include "test.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_packet_new (void)
{
  mongo_packet *p;

  ok ((p = mongo_wire_packet_new ()) != NULL,
      "mongo_wire_packet_new() works");
  lives_ok ({ mongo_wire_packet_free (NULL); },
	    "mongo_wire_packet_free(NULL) works");
  mongo_wire_packet_free (p);
  pass ("mongo_wire_packet_free() works");
}

RUN_TEST (3, mongo_wire_packet_new);
