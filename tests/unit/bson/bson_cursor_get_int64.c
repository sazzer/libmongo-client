#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_int64 (void)
{
  bson *b;
  bson_cursor *c;
  gint64 d = (gint64)987654;

  ok (bson_cursor_get_int64 (NULL, &d) == FALSE,
      "bson_cursor_get_int64() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_int64 (c, NULL) == FALSE,
      "bson_cursor_get_int64() with a NULL destination fails");
  ok (bson_cursor_get_int64 (c, &d) == FALSE,
      "bson_cursor_get_int64() at the initial position fails");
  cmp_ok (d, "==", 987654,
	  "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "int64");
  ok (bson_cursor_get_int64 (c, &d),
      "bson_cursor_get_int64() works");
  cmp_ok (d, "==", (gint64)-42,
	  "bson_cursor_get_int64() returns the correct result");

  bson_cursor_free (c);

  c = bson_find (b, "double");
  ok (bson_cursor_get_int64 (c, &d) == FALSE,
      "bson_cursor_get_int64() should fail when the cursor points to "
      "non-int64 data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_int64);
