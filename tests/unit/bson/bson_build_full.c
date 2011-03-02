#include "bson.h"
#include "tap.h"
#include "test.h"
#include "test-generator.h"

#include <string.h>
#include <glib.h>

void
test_bson_build_full (void)
{
  bson *d, *o;

  d = bson_build_full
    (BSON_TYPE_DOCUMENT, "user", TRUE, bson_build
     (BSON_TYPE_STRING, "name", "V.A. Lucky", -1,
      BSON_TYPE_INT32, "id", 12345,
      BSON_TYPE_NONE),
     BSON_TYPE_ARRAY, "posts", TRUE, bson_build_full
     (BSON_TYPE_DOCUMENT, "0", TRUE, bson_build_full
      (BSON_TYPE_STRING, "title", FALSE, "Post #1", -1,
       BSON_TYPE_UTC_DATETIME, "date", FALSE, 1294860709000,
       BSON_TYPE_ARRAY, "comments", TRUE, bson_build
       (BSON_TYPE_STRING, "0", "first!", -1,
	BSON_TYPE_STRING, "1", "2nd!", -1,
	BSON_TYPE_STRING, "2", "last!", -1,
	BSON_TYPE_NONE),
       BSON_TYPE_NONE),
      BSON_TYPE_DOCUMENT, "1", TRUE, bson_build
      (BSON_TYPE_STRING, "title", "Post #2", -1,
       BSON_TYPE_UTC_DATETIME, "date", 1294860709000,
       BSON_TYPE_NONE),
      BSON_TYPE_NONE),
     BSON_TYPE_NONE);
  bson_finish (d);

  o = test_bson_generate_nested ();

  cmp_ok (bson_size (d), "==", bson_size (o),
	  "bson_build_full() and hand crafted BSON object sizes match");

  ok (memcmp (bson_data (d), bson_data (o), bson_size (d)) == 0,
      "bson_build_full() and hand crafted BSON objects match");

  bson_free (d);
  bson_free (o);

  d = bson_build_full (BSON_TYPE_UNDEFINED, "undef", FALSE,
		       BSON_TYPE_NONE);
  ok (d == NULL,
      "bson_build_full() should fail with an unsupported element type");
  d = bson_build_full (BSON_TYPE_STRING, "str", FALSE, "hello", -1,
		       BSON_TYPE_UNDEFINED, "undef", FALSE,
		       BSON_TYPE_NONE);
  ok (d == NULL,
      "bson_build_full() should fail with an unsupported element type");

}

RUN_TEST (4, bson_build_full);
