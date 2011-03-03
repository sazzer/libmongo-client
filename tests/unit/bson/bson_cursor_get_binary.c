#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_binary (void)
{
  bson *b;
  bson_cursor *c;
  const guint8 *d = (guint8 *)"deadbeef";
  bson_binary_subtype t = 0xff;
  gint32 s = -1;

  ok (bson_cursor_get_binary (NULL, &t, &d, &s) == FALSE,
      "bson_cursor_get_binary() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_binary (c, NULL, NULL, NULL) == FALSE,
      "bson_cursor_get_binary() with NULL destinations fails");
  ok (bson_cursor_get_binary (c, NULL, &d, &s) == FALSE,
      "bson_cursor_get_binary() with a NULL subtype destination fails");
  ok (bson_cursor_get_binary (c, &t, NULL, &s) == FALSE,
      "bson_cursor_get_binary() with a NULL binary destination fails");
  ok (bson_cursor_get_binary (c, &t, &d, NULL) == FALSE,
      "bson_cursor_get_binary() with a NULL size destination fails");
  ok (bson_cursor_get_binary (c, &t, &d, &s) == FALSE,
      "bson_cursor_get_binary() at the initial position fails");
  ok (memcmp (d, "deadbeef", sizeof ("deadbeef")) == 0,
      "binary destination remains unchanged after failed cursor operations");
  cmp_ok (t, "==", 0xff,
	  "subtype destination remains unchanged after failed cursor "
	  "operations");
  cmp_ok (s, "==", -1,
	  "size destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "binary0");
  ok (bson_cursor_get_binary (c, &t, &d, &s),
      "bson_cursor_get_binary() works");
  cmp_ok (s, "==", 7,
	  "bson_cursor_get_binary() returns the correct result");
  ok (memcmp (d, "foo\0bar", s) == 0,
      "bson_cursor_get_binary() returns the correct result");
  cmp_ok (t, "==", BSON_BINARY_SUBTYPE_GENERIC,
      "bson_cursor_get_binary() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_binary (c, &t, &d, &s) == FALSE,
      "bson_cursor_get_binary() should fail when the cursor points to "
      "non-binary data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (14, bson_cursor_get_binary);
