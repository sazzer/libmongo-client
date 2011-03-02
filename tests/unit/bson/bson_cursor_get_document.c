#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_document (void)
{
  bson *b, *d = NULL;
  bson_cursor *c;

  ok (bson_cursor_get_document (NULL, &d) == FALSE,
      "bson_cursor_get_document() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_document (c, NULL) == FALSE,
      "bson_cursor_get_document() with a NULL destination fails");
  ok (bson_cursor_get_document (c, &d) == FALSE,
      "bson_cursor_get_document() at the initial position fails");
  ok (d == NULL,
      "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "doc");
  ok (bson_cursor_get_document (c, &d),
      "bson_cursor_get_document() works");
  cmp_ok (bson_size (d), ">", 0,
	  "the returned document is finished");
  bson_free (d);

  bson_cursor_next (c);
  ok (bson_cursor_get_document (c, &d) == FALSE,
      "bson_cursor_get_document() fails if the cursor points to "
      "non-document data");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (7, bson_cursor_get_document);
