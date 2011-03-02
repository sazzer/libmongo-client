#include "bson.h"
#include "tap.h"
#include "test.h"

#include <string.h>
#include <glib.h>

void
test_bson_build (void)
{
  bson *b, *o; //, *d, *a;

  //a = bson_new ();
  //bson_append_int32 (a, "0", 32,
  
  b = o = test_bson_generate_full ();

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
