#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_string (void)
{
  bson *b;
  bson_cursor *c;
  const gchar *s = "deadbeef";

  ok (bson_cursor_get_string (NULL, &s) == FALSE,
      "bson_cursor_get_string() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_string (c, NULL) == FALSE,
      "bson_cursor_get_string() with a NULL destination fails");
  ok (bson_cursor_get_string (c, &s) == FALSE,
      "bson_cursor_get_string() at the initial position fails");
  is (s, "deadbeef",
      "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "str");
  ok (bson_cursor_get_string (c, &s),
      "bson_cursor_get_string() works");
  is (s, "hello world",
      "bson_cursor_get_string() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_string (c, &s) == FALSE,
      "bson_cursor_get_string() should fail when the cursor points to "
      "non-string data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_string);
