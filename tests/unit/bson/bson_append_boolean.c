#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_boolean (void)
{
  bson *b;

  b = bson_new ();
  ok (bson_append_boolean (b, "FALSE", FALSE),
      "bson_append_boolean() works");
  ok (bson_append_boolean (b, "TRUE", TRUE),
      "bson_append_boolean() works still");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 20, "BSON boolean element size check");
  ok (memcmp (bson_data (b),
	      "\024\000\000\000\010\106\101\114\123\105\000\000\010\124\122"
	      "\125\105\000\001\000",
	      bson_size (b)) == 0,
      "BSON boolean element contents check");

  bson_free (b);

  b = bson_new ();
  ok (bson_append_boolean (b, NULL, TRUE) == FALSE,
      "bson_append_boolean() with a NULL key should fail");
  ok (bson_append_boolean (NULL, "TRUE", TRUE) == FALSE,
      "bson_append_boolean() without a BSON object should fail");
  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  ok (bson_append_boolean (b, "b", TRUE) == FALSE,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (8, bson_boolean);
