#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_type (void)
{
  bson *b;
  bson_cursor *c;

  cmp_ok (bson_cursor_type (NULL), "==", BSON_TYPE_NONE,
	  "bson_cursor_type(NULL) should fail");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_NONE,
	  "bson_cursor_type() should fail at the beginning of the BSON "
	  "object");
  bson_cursor_next (c);
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_DOUBLE,
	  "bson_cursor_type() works");

  bson_cursor_free (c);
  bson_free (b);
}

RUN_TEST (3, bson_cursor_type);
