#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_double (void)
{
  bson *b;
  double d = 3.14;

  b = bson_new ();
  ok (bson_append_double (b, "double", d), "bson_append_double() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 21, "BSON double element size check");
  ok (memcmp (bson_data (b),
	      "\025\000\000\000\001\144\157\165\142\154\145\000\037\205\353"
	      "\121\270\036\011\100\000",
	      bson_size (b)) == 0,
      "BSON double element contents check");

  bson_free (b);

  b = bson_new ();
  ok (bson_append_double (b, NULL, d) == FALSE,
      "bson_append_double() with a NULL key should fail");
  ok (bson_append_double (NULL, "double", d) == FALSE,
      "bson_append_double() without a BSON object should fail");
  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  ok (bson_append_double (b, "d", d) == FALSE,
      "Appending to a finished element should fail");

  bson_free (b);
}

RUN_TEST (7, bson_double);
