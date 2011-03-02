#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_utc_datetime (void)
{
  bson *b;

  b = bson_new ();
  ok (bson_append_utc_datetime (b, "date", 1294860709000),
      "bson_append_utc_datetime() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 19, "BSON UTC datetime element size check");
  ok (memcmp (bson_data (b),
	      "\023\000\000\000\011\144\141\164\145\000\210\154\266\173\055"
	      "\001\000\000\000",
	      bson_size (b)) == 0,
      "BSON UTC datetime element contents check");

  bson_free (b);

  b = bson_new ();
  ok (bson_append_utc_datetime (b, NULL, 1294860709000) == FALSE,
      "bson_append_utc_datetime() with a NULL key should fail");
  ok (bson_append_utc_datetime (NULL, "date", 1294860709000) == FALSE,
      "bson_append_utc_datetime() without a BSON object should fail");
  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");
  bson_free (b);
}

RUN_TEST (6, bson_utc_datetime);
