#include "tap.h"
#include "test.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_packet_get_set_data (void)
{
  mongo_packet *p;
  mongo_packet_header h;
  guint8 data[32768];
  const guint8 *idata;

  p = mongo_wire_packet_new ();
  memset (data, 'x', sizeof (data));

  ok (mongo_wire_packet_get_data (NULL, &idata) == -1,
      "mongo_wire_packet_get_data() with a NULL packet should fail");
  ok (mongo_wire_packet_get_data (p, NULL) == -1,
      "mongo_wire_packet_get_data() with NULL destination should fail");
  ok (mongo_wire_packet_set_data (NULL, (const guint8 *)&data,
				  sizeof (data)) == FALSE,
      "mongo_wire_packet_set_data() with a NULL packet should fail");
  ok (mongo_wire_packet_set_data (p, NULL, sizeof (data)) == FALSE,
      "mongo_wire_packet_set_data() with NULL data should fail");
  ok (mongo_wire_packet_set_data (p, (const guint8 *)&data, 0) == FALSE,
      "mongo_wire_packet_set_data() with zero size should fail");
  ok (mongo_wire_packet_set_data (p, (const guint8 *)&data, -1) == FALSE,
      "mongo_wire_packet_set_data() with negative size should fail");

  ok (mongo_wire_packet_set_data (p, (const guint8 *)&data,
				  sizeof (data)),
      "mongo_wire_packet_set_data() works");
  cmp_ok (mongo_wire_packet_get_data (p, &idata), "==", sizeof (data),
	  "mongo_wire_packet_get_data() works");

  mongo_wire_packet_get_header (p, &h);

  cmp_ok (h.length, "==", sizeof (data) + sizeof (mongo_packet_header),
	  "Packet length is updated properly");
  ok (memcmp (data, idata, sizeof (data)) == 0,
      "Setting & retrieving data works");

  memset (data, 'a', sizeof (data));

  ok (mongo_wire_packet_set_data (p, (const guint8 *)&data,
				  sizeof (data) / 2),
      "Re-setting the data works");
  cmp_ok (mongo_wire_packet_get_data (p, &idata), "==", sizeof (data) / 2,
	  "Retrieving the data works still");

  mongo_wire_packet_get_header (p, &h);

  cmp_ok (h.length, "==", sizeof (data) / 2 + sizeof (mongo_packet_header),
	  "Packet length is updated properly");
  ok (memcmp (data, idata, sizeof (data) / 2) == 0,
      "Setting & retrieving data works");

  mongo_wire_packet_free (p);
}

RUN_TEST (14, mongo_wire_packet_get_set_data);
