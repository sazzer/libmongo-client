#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_regex (void)
{
  bson *b;
  bson_cursor *c;
  const gchar *r = "deadbeef";
  const gchar *o = "g";

  ok (bson_cursor_get_regex (NULL, &r, &o) == FALSE,
      "bson_cursor_get_regex() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_regex (c, NULL, NULL) == FALSE,
      "bson_cursor_get_regex() with NULL destinations fails");
  ok (bson_cursor_get_regex (c, &r, NULL) == FALSE,
      "bson_cursor_get_regex() with a NULL option destination fails");
  ok (bson_cursor_get_regex (c, NULL, &o) == FALSE,
      "bson_cursor_get_regex() with a NULL regex destination fails");
  ok (bson_cursor_get_regex (c, &r, &o) == FALSE,
      "bson_cursor_get_regex() at the initial position fails");
  is (r, "deadbeef",
      "regex destination remains unchanged after failed cursor operations");
  is (o, "g",
      "options destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "foobar");
  ok (bson_cursor_get_regex (c, &r, &o),
      "bson_cursor_get_regex() works");
  is (r, "s/foo.*bar/",
      "bson_cursor_get_regex() returns the correct result");
  is (o, "i",
      "bson_cursor_get_regex() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_regex (c, &r, &o) == FALSE,
      "bson_cursor_get_regex() should fail when the cursor points to "
      "non-regex data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (11, bson_cursor_get_regex);
