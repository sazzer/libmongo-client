#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_javascript (void)
{
  bson *b;
  bson_cursor *c;
  const gchar *s = "deadbeef";

  ok (bson_cursor_get_javascript (NULL, &s) == FALSE,
      "bson_cursor_get_javascript() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_javascript (c, NULL) == FALSE,
      "bson_cursor_get_javascript() with a NULL destination fails");
  ok (bson_cursor_get_javascript (c, &s) == FALSE,
      "bson_cursor_get_javascript() at the initial position fails");
  is (s, "deadbeef",
      "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "alert");
  ok (bson_cursor_get_javascript (c, &s),
      "bson_cursor_get_javascript() works");
  is (s, "alert (\"hello world!\");",
      "bson_cursor_get_javascript() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_javascript (c, &s) == FALSE,
      "bson_cursor_get_javascript() should fail when the cursor points to "
      "non-javascript data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_javascript);
