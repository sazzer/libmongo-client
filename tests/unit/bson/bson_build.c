#include "bson.h"
#include "tap.h"
#include "test.h"

#include <string.h>
#include <glib.h>

void
test_bson_build (void)
{
  bson *b, *o, *d, *a;
  guint8 oid[] = "1234567890ab";

  a = bson_build (BSON_TYPE_INT32, "0", 32,
		  BSON_TYPE_INT64, "1", (gint64)-42,
		  BSON_TYPE_NONE);
  bson_finish (a);
  d = bson_build (BSON_TYPE_STRING, "name", "sub-document", -1,
		  BSON_TYPE_INT32, "answer", 42,
		  BSON_TYPE_NONE);
  bson_finish (d);

  b = bson_build (BSON_TYPE_DOUBLE, "double", 3.14,
		  BSON_TYPE_STRING, "str", "hello world", -1,
		  BSON_TYPE_DOCUMENT, "doc", d,
		  BSON_TYPE_ARRAY, "array", a,
		  BSON_TYPE_BINARY, "binary0", BSON_BINARY_SUBTYPE_GENERIC,
		  (guint8 *)"foo\0bar", 7,
		  BSON_TYPE_OID, "_id", oid,
		  BSON_TYPE_BOOLEAN, "TRUE", FALSE,
		  BSON_TYPE_UTC_DATETIME, "date", 1294860709000,
		  BSON_TYPE_NULL, "null",
		  BSON_TYPE_REGEXP, "foobar", "s/foo.*bar/", "i",
		  BSON_TYPE_JS_CODE, "alert", "alert (\"hello world!\");", -1,
		  BSON_TYPE_SYMBOL, "sex", "Marylin Monroe", -1,
		  BSON_TYPE_INT32, "int32", 32,
		  BSON_TYPE_INT64, "int64", (gint64)-42,
		  BSON_TYPE_NONE);
  bson_finish (b);

  o = test_bson_generate_full ();

  cmp_ok (bson_size (b), "==", bson_size (o),
	  "bson_build() and hand crafted BSON object sizes match");
  ok (memcmp (bson_data (b), bson_data (o), bson_size (b)) == 0,
      "bson_build() and hand crafted BSON objects match");

  bson_free (b);
  bson_free (o);

  b = bson_build (BSON_TYPE_UNDEFINED, BSON_TYPE_NONE);
  ok (b == NULL,
      "bson_build() should fail with an unsupported element type");
  b = bson_build (BSON_TYPE_STRING, "str", "hello", -1,
		  BSON_TYPE_UNDEFINED,
		  BSON_TYPE_NONE);
  ok (b == NULL,
      "bson_build() should fail with an unsupported element type");
}

RUN_TEST (4, bson_build);
