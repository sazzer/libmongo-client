#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_timestamp (void)
{
  bson *b;
  gint64 l = 9876543210;

  b = bson_new ();
  ok (bson_append_timestamp (b, "ts", l), "bson_append_timestamp() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 17, "BSON timestamp element size check");
  ok (memcmp (bson_data (b),
	      "\021\000\000\000\021\164\163\000\352\026\260\114\002\000\000"
	      "\000\000",
	      bson_size (b)) == 0,
      "BSON timestamp element contents check");

  bson_free (b);
}

RUN_TEST (3, bson_timestamp);
