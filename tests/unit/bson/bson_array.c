#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_array (void)
{
  bson *b, *e1, *e2;

  e1 = bson_new ();
  bson_append_int32 (e1, "0", 1984);
  bson_append_string (e1, "1", "hello world", -1);
  bson_finish (e1);

  e2 = bson_new ();
  bson_append_string (e2, "0", "bar", -1);
  ok (bson_append_array (e2, "1", e1),
      "bson_append_array() works");
  bson_finish (e2);
  bson_free (e1);

  b = bson_new ();
  ok (bson_append_array (b, "0", e2),
      "bson_append_array() works still");
  bson_finish (b);
  bson_free (e2);

  cmp_ok (bson_size (b), "==", 58, "BSON array element size check");
  ok (memcmp (bson_data (b),
	      "\072\000\000\000\004\060\000\062\000\000\000\002\060\000\004"
	      "\000\000\000\142\141\162\000\004\061\000\037\000\000\000\020"
	      "\060\000\300\007\000\000\002\061\000\014\000\000\000\150\145"
	      "\154\154\157\040\167\157\162\154\144\000\000\000\000",
	      bson_size (b)) == 0,
      "BSON array element contents check");

  bson_free (b);
}

RUN_TEST (4, bson_array);
