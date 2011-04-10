#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_find (void)
{
  bson *b;
  bson_cursor *c;

  ok (bson_find (NULL, NULL) == NULL,
      "bson_find() with NULL parameters should fail");
  ok (bson_find (NULL, "key") == NULL,
      "bson_find() with a NULL BSON object should fail");
  b = bson_new ();
  ok (bson_find (b, "key") == NULL,
      "bson_find() with an unfinished BSON object should fail");
  bson_free (b);

  b = test_bson_generate_full ();
  ok (bson_find (b, NULL) == FALSE,
      "bson_find() with a NULL key should fail");
  ok (bson_find (b, "__invalid__") == FALSE,
      "bson_find() with a non-existent key should fail");
  ok ((c = bson_find (b, "alert")) != NULL,
      "bson_find() works");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (6, bson_find);
