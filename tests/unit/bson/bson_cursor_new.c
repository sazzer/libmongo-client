#include "tap.h"
#include "test.h"
#include "test-generator.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_new (void)
{
  bson *b;
  bson_cursor *c;

  ok (bson_cursor_new (NULL) == NULL,
      "bson_cursor_new(NULL) should fail");

  b = bson_new ();
  ok (bson_cursor_new (b) == NULL,
      "bson_cursor_new() should fail with an unfinished BSON object");
  bson_free (b);

  b = test_bson_generate_flat ();
  ok ((c = bson_cursor_new (b)) != NULL,
      "bson_cursor_new() works");
  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (3, bson_cursor_new);
