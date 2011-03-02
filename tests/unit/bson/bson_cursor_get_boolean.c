#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_boolean (void)
{
  bson *b;
  bson_cursor *c;
  gboolean d = TRUE;

  ok (bson_cursor_get_boolean (NULL, &d) == FALSE,
      "bson_cursor_get_boolean() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_boolean (c, NULL) == FALSE,
      "bson_cursor_get_boolean() with a NULL destination fails");
  ok (bson_cursor_get_boolean (c, &d) == FALSE,
      "bson_cursor_get_boolean() at the initial position fails");
  cmp_ok (d, "==", TRUE,
	  "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "TRUE");
  ok (bson_cursor_get_boolean (c, &d),
      "bson_cursor_get_boolean() works");
  cmp_ok (d, "==", FALSE,
	  "bson_cursor_get_boolean() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_boolean (c, &d) == FALSE,
      "bson_cursor_get_boolean() should fail when the cursor points to "
      "non-double data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_boolean);
