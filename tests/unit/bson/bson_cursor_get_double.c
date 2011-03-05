#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_double (void)
{
  bson *b;
  bson_cursor *c;
  gdouble d = 12.34;

  ok (bson_cursor_get_double (NULL, &d) == FALSE,
      "bson_cursor_get_double() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_double (c, NULL) == FALSE,
      "bson_cursor_get_double() with a NULL destination fails");
  ok (bson_cursor_get_double (c, &d) == FALSE,
      "bson_cursor_get_double() at the initial position fails");
  cmp_ok (d, "==", 12.34,
	  "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "double");
  ok (bson_cursor_get_double (c, &d),
      "bson_cursor_get_double() works");
  cmp_ok (d, "==", 3.14,
	  "bson_cursor_get_double() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_double (c, &d) == FALSE,
      "bson_cursor_get_double() should fail when the cursor points to "
      "non-double data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_double);
