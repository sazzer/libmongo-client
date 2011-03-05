#include "bson.h"
#include "tap.h"
#include "test.h"

#include <string.h>
#include <glib.h>

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void
test_bson_build_full (void)
{
  bson *b, *o;

  b = bson_build_full (BSON_TYPE_DOUBLE, "double", FALSE, 3.14,
		       BSON_TYPE_STRING, "str", FALSE, "hello world", -1,
		       BSON_TYPE_DOCUMENT, "doc", TRUE,
		       bson_build (BSON_TYPE_STRING, "name", "sub-document", -1,
				   BSON_TYPE_INT32, "answer", 42,
				   BSON_TYPE_NONE),
		       BSON_TYPE_ARRAY, "array", TRUE,
		       bson_build (BSON_TYPE_INT32, "0", 32,
				   BSON_TYPE_INT64, "1", (gint64)-42,
				   BSON_TYPE_NONE),
		       BSON_TYPE_BINARY, "binary0", FALSE, BSON_BINARY_SUBTYPE_GENERIC,
		       "foo\0bar", 7,
		       BSON_TYPE_OID, "_id", FALSE, "1234567890ab",
		       BSON_TYPE_BOOLEAN, "TRUE", FALSE, FALSE,
		       BSON_TYPE_UTC_DATETIME, "date", FALSE, 1294860709000,
		       BSON_TYPE_TIMESTAMP, "ts", FALSE, 1294860709000,
		       BSON_TYPE_NULL, "null", FALSE,
		       BSON_TYPE_REGEXP, "foobar", FALSE, "s/foo.*bar/", "i",
		       BSON_TYPE_JS_CODE, "alert", FALSE, "alert (\"hello world!\");", -1,
		       BSON_TYPE_SYMBOL, "sex", FALSE, "Marilyn Monroe", -1,
		       BSON_TYPE_INT32, "int32", FALSE, 32,
		       BSON_TYPE_INT64, "int64", FALSE, (gint64)-42,
		       BSON_TYPE_NONE);
  bson_finish (b);

  o = test_bson_generate_full ();

  cmp_ok (bson_size (b), "==", bson_size (o),
	  "bson_build_full() and hand crafted BSON object sizes match");

  ok (memcmp (bson_data (b), bson_data (o), bson_size (b)) == 0,
      "bson_build_full() and hand crafted BSON objects match");

  bson_free (b);
  bson_free (o);

  b = bson_build_full (BSON_TYPE_UNDEFINED, "undef", FALSE,
		       BSON_TYPE_NONE);
  ok (b == NULL,
      "bson_build_full() should fail with an unsupported element type");
  b = bson_build_full (BSON_TYPE_STRING, "str", FALSE, "hello", -1,
		       BSON_TYPE_UNDEFINED, "undef", FALSE,
		       BSON_TYPE_NONE);
  ok (b == NULL,
      "bson_build_full() should fail with an unsupported element type");

}

RUN_TEST (4, bson_build_full);
