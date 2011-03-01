#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_null (void)
{
  bson *b;

  b = bson_new ();
  ok (bson_append_null (b, "null"),
      "bson_append_null() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 11, "BSON NULL element size check");
  ok (memcmp (bson_data (b),
	      "\013\000\000\000\012\156\165\154\154\000\000",
	      bson_size (b)) == 0,
      "BSON NULL element contents check");

  bson_free (b);
}

RUN_TEST (3, bson_null);
