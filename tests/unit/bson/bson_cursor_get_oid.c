#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_oid (void)
{
  bson *b;
  bson_cursor *c;
  const gchar *s = "abababababab";

  ok (bson_cursor_get_oid (NULL, (const guint8 **)&s) == FALSE,
      "bson_cursor_get_oid() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_oid (c, NULL) == FALSE,
      "bson_cursor_get_oid() with a NULL destination fails");
  ok (bson_cursor_get_oid (c, (const guint8 **)&s) == FALSE,
      "bson_cursor_get_oid() at the initial position fails");
  ok (memcmp (s, "abababababab", 12) == 0,
      "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "_id");
  ok (bson_cursor_get_oid (c, (const guint8 **)&s),
      "bson_cursor_get_oid() works");
  ok (memcmp (s, "1234567890ab", 12) == 0,
      "bson_cursor_get_oid() returns the correct result");

  bson_cursor_next (c);
  ok (bson_cursor_get_oid (c, (const guint8 **)&s) == FALSE,
      "bson_cursor_get_oid() should fail when the cursor points to "
      "non-oid data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_oid);
