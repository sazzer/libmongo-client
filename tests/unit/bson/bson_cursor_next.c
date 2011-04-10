#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_next (void)
{
  bson *b;
  bson_cursor *c;

  ok (bson_cursor_next (NULL) == FALSE,
      "bson_cursor_next (NULL) should fail");

  b = bson_new ();
  bson_finish (b);
  c = bson_cursor_new (b);

  ok (bson_cursor_next (c) == FALSE,
      "bson_cursor_next() should fail with an empty document");

  bson_cursor_free (c);
  bson_free (b);

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);
  ok (bson_cursor_next (c),
      "initial bson_cursor_next() works");
  ok (bson_cursor_next (c),
      "subsequent bson_cursor_next() works too");

  while (bson_cursor_next (c)) ;

  ok (bson_cursor_next (c) == FALSE,
      "bson_cursor_next() fails after the end of the BSON object");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (5, bson_cursor_next);
