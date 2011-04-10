#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_int64 (void)
{
  bson *b;
  gint64 l = 9876543210;

  b = bson_new ();
  ok (bson_append_int64 (b, "i64", l), "bson_append_int64() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 18, "BSON int64 element size check");
  ok (memcmp (bson_data (b),
	      "\022\000\000\000\022\151\066\064\000\352\026\260\114\002\000"
	      "\000\000\000",
	      bson_size (b)) == 0,
      "BSON int64 element contents check");

  bson_free (b);

  b = bson_new ();
  ok (bson_append_int64 (b, NULL, l) == FALSE,
      "bson_append_int64() with a NULL key should fail");
  ok (bson_append_int64 (NULL, "i64", l) == FALSE,
      "bson_append_int64() without a BSON object should fail");
  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  ok (bson_append_int64 (b, "i64", l) == FALSE,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (7, bson_int64);
