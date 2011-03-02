#include "bson.h"
#include "tap.h"
#include "test.h"
#include "test-generator.h"

#include <string.h>
#include <glib.h>

void
test_bson_build (void)
{
  bson *d, *o;
  bson *user, *comments, *p1, *p2, *posts;

  user = bson_build (BSON_TYPE_STRING, "name", "V.A. Lucky", -1,
		     BSON_TYPE_INT32, "id", 12345,
		     BSON_TYPE_NONE);
  bson_finish (user);

  comments = bson_build (BSON_TYPE_STRING, "0", "first!", -1,
			 BSON_TYPE_STRING, "1", "2nd!", -1,
			 BSON_TYPE_STRING, "2", "last!", -1,
			 BSON_TYPE_NONE);
  bson_finish (comments);

  p1 = bson_build (BSON_TYPE_STRING, "title", "Post #1", -1,
		   BSON_TYPE_UTC_DATETIME, "date", 1294860709000,
		   BSON_TYPE_ARRAY, "comments", comments,
		   BSON_TYPE_NONE);
  bson_finish (p1);

  p2 = bson_build (BSON_TYPE_STRING, "title", "Post #2", -1,
		   BSON_TYPE_UTC_DATETIME, "date", 1294860709000,
		   BSON_TYPE_NONE);
  bson_finish (p2);

  posts = bson_build (BSON_TYPE_DOCUMENT, "0", p1,
		      BSON_TYPE_DOCUMENT, "1", p2,
		      BSON_TYPE_NONE);
  bson_finish (posts);

  d = bson_build (BSON_TYPE_DOCUMENT, "user", user,
		  BSON_TYPE_ARRAY, "posts", posts,
		  BSON_TYPE_NONE);
  bson_finish (d);

  bson_free (user);
  bson_free (comments);
  bson_free (p1);
  bson_free (p2);
  bson_free (posts);

  o = test_bson_generate_nested ();

  cmp_ok (bson_size (d), "==", bson_size (o),
	  "bson_build() and hand crafted BSON object sizes match");
  ok (memcmp (bson_data (d), bson_data (o), bson_size (d)) == 0,
      "bson_build() and hand crafted BSON objects match");

  bson_free (d);
  bson_free (o);

  d = bson_build (BSON_TYPE_UNDEFINED, BSON_TYPE_NONE);
  ok (d == NULL,
      "bson_build() should fail with an unsupported element type");
  d = bson_build (BSON_TYPE_STRING, "str", "hello", -1,
		  BSON_TYPE_UNDEFINED,
		  BSON_TYPE_NONE);
  ok (d == NULL,
      "bson_build() should fail with an unsupported element type");
}

RUN_TEST (4, bson_build);
