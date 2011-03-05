#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_int32 (void)
{
  bson *b;
  bson_cursor *c;
  gint d = 12345;

  ok (bson_cursor_get_int32 (NULL, &d) == FALSE,
      "bson_cursor_get_int32() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_int32 (c, NULL) == FALSE,
      "bson_cursor_get_int32() with a NULL destination fails");
  ok (bson_cursor_get_int32 (c, &d) == FALSE,
      "bson_cursor_get_int32() at the initial position fails");
  cmp_ok (d, "==", 12345,
	  "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "int32");
  ok (bson_cursor_get_int32 (c, &d),
      "bson_cursor_get_int32() works");
  cmp_ok (d, "==", 32,
	  "bson_cursor_get_int32() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_int32 (c, &d) == FALSE,
      "bson_cursor_get_int32() should fail when the cursor points to "
      "non-int32 data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_int32);
