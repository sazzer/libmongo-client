#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_regex (void)
{
  bson *b;

  b = bson_new ();
  ok (bson_append_regex (b, "regex", "foo.*bar", "i"),
      "bson_append_regex() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 23, "BSON regex element size check");
  ok (memcmp (bson_data (b),
	      "\027\000\000\000\013\162\145\147\145\170\000\146\157\157\056"
	      "\052\142\141\162\000\151\000\000",
	      bson_size (b)) == 0,
      "BSON regex element contents check");

  bson_free (b);
}

RUN_TEST (3, bson_regex);
