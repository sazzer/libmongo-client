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

  b = bson_new ();
  ok (bson_append_regex (b, "regex", "foo.*bar", NULL) == FALSE,
      "bson_append_regex() without options should fail");
  ok (bson_append_regex (b, "regex", NULL, "i") == FALSE,
      "bson_append_regex() without a regex should fail");
  ok (bson_append_regex (b, NULL, "foo.*bar", "i") == FALSE,
      "bson_append_regex() should fail without a key name");
  ok (bson_append_regex (NULL, "regex", "foo.*bar", "i") == FALSE,
      "bson_append_regex() should fail without a BSON object");
  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");
  bson_free (b);
}

RUN_TEST (8, bson_regex);
