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

  b = bson_new ();
  ok (bson_append_binary (b, NULL, BSON_BINARY_SUBTYPE_GENERIC,
			  (guint8 *)"foo\0bar", 7) == FALSE,
      "bson_append_binary() without a key name should fail");
  ok (bson_append_binary (b, "binary1", BSON_BINARY_SUBTYPE_GENERIC,
			  NULL, 10) == FALSE,
      "bson_append_binary () without binary data should fail");
  ok (bson_append_binary (b, "binary3", BSON_BINARY_SUBTYPE_GENERIC,
			  (guint8 *)"foo\0bar", -1) == FALSE,
      "bson_append_binary () with an invalid length should fail");
  ok (bson_append_binary (NULL, "binary1", BSON_BINARY_SUBTYPE_GENERIC,
			  (guint8 *)"foo\0bar", 7) == FALSE,
      "bson_append_binary () without a BSON object should fail");
  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");
  bson_free (b);
}

RUN_TEST (9, bson_binary);
