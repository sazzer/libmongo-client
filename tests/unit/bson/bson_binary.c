#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_binary (void)
{
  bson *b;

  b = bson_new ();
  ok (bson_append_binary (b, "binary0", BSON_BINARY_SUBTYPE_GENERIC,
			  (guint8 *)"foo\0bar", 7),
      "bson_append_binary(), type 0 works");
  ok (bson_append_binary (b, "binary2", BSON_BINARY_SUBTYPE_BINARY,
			  (guint8 *)"\0\0\0\7foo\0bar", 11),
      "bson_append_binary(), type 2 works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 51, "BSON binary element size check");
  ok (memcmp (bson_data (b),
	      "\063\000\000\000\005\142\151\156\141\162\171\060\000\007\000"
	      "\000\000\000\146\157\157\000\142\141\162\005\142\151\156\141"
	      "\162\171\062\000\013\000\000\000\002\000\000\000\007\146\157"
	      "\157\000\142\141\162\000",
	      bson_size (b)) == 0,
      "BSON binary element contents check");

  bson_free (b);
}

RUN_TEST (4, bson_binary);
