#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_empty (void)
{
  bson *b;

  b = bson_new ();
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 5, "Empty BSON size check");
  ok (memcmp (bson_data (b), "\005\000\000\000\000", bson_size (b)) == 0,
      "Empty BSON contents check");

  bson_free (b);
}

RUN_TEST (2, bson_empty)
