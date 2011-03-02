#include "test.h"
#include "bson.h"

#include <glib.h>
#include <string.h>

bson *
test_bson_generate_full (void)
{
  bson *b, *d, *a;

  a = bson_build (BSON_TYPE_INT32, "0", 32,
		  BSON_TYPE_INT64, "1", -42,
		  BSON_TYPE_NONE);
  d = bson_build (BSON_TYPE_STRING, "name", "sub-document", -1,
		  BSON_TYPE_INT32, "answer", 42,
		  BSON_TYPE_NONE);

  b = bson_build (BSON_TYPE_DOUBLE, "double", 3.14,
		  BSON_TYPE_STRING, "str", "hello world", -1,
		  BSON_TYPE_DOCUMENT, "doc", d,
		  BSON_TYPE_ARRAY, "array", a,
		  BSON_TYPE_BINARY, "binary0", BSON_BINARY_SUBTYPE_GENERIC,
		  "foo\0bar", 7,
		  BSON_TYPE_OID, "_id", "0123456789abcd",
		  BSON_TYPE_BOOLEAN, "TRUE", FALSE,
		  BSON_TYPE_UTC_DATETIME, "date", 1294860709000,
		  BSON_TYPE_NULL, "null",
		  BSON_TYPE_REGEXP, "foobar", "s/foo.*bar/", "i",
		  BSON_TYPE_JS_CODE, "alert", "alert (\"hello world!\");", -1,
		  BSON_TYPE_SYMBOL, "sex", "Marylin Monroe", -1,
		  BSON_TYPE_INT32, "int32", 32,
		  BSON_TYPE_INT64, "int64", 42,
		  BSON_TYPE_NONE);

  bson_free (d);
  bson_free (a);

  bson_finish (b);
  return b;
}
