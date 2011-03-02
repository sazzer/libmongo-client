#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_timestamp (void)
{
  bson *b;
  bson_cursor *c;
  gint64 d = (gint64)987654;

  ok (bson_cursor_get_timestamp (NULL, &d) == FALSE,
      "bson_cursor_get_timestamp() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_timestamp (c, NULL) == FALSE,
      "bson_cursor_get_timestamp() with a NULL destination fails");
  ok (bson_cursor_get_timestamp (c, &d) == FALSE,
      "bson_cursor_get_timestamp() at the initial position fails");
  cmp_ok (d, "==", 987654,
	  "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "ts");
  ok (bson_cursor_get_timestamp (c, &d),
      "bson_cursor_get_timestamp() works");
  ok (d == 1294860709000,
      "bson_cursor_get_timestamp() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_timestamp (c, &d) == FALSE,
      "bson_cursor_get_timestamp() should fail when the cursor points to "
      "non-timestamp data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_timestamp);
