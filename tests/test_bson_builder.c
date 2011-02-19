#include "bson.h"
#include "test.h"
#include "test-generator.h"

#include <string.h>
#include <glib.h>

void
test_bson_builder (void)
{
  bson *d, *o;
  bson *user, *comments, *p1, *p2, *posts;

  TEST (bson_builder.simple);
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

  g_assert_cmpint (bson_size (d), ==, bson_size (o));

  g_assert (memcmp (bson_data (d), bson_data (o), bson_size (d)) == 0);

  bson_free (d);
  bson_free (o);

  PASS ();
}

void
test_bson_builder_full (void)
{
  bson *d, *o;

  TEST (bson_builder.full);
  d = bson_build_full (BSON_TYPE_DOCUMENT, "user", TRUE, bson_build (BSON_TYPE_STRING, "name", "V.A. Lucky", -1,
								     BSON_TYPE_INT32, "id", 12345,
								     BSON_TYPE_NONE),
		       BSON_TYPE_ARRAY, "posts", TRUE, bson_build_full (BSON_TYPE_DOCUMENT, "0", TRUE, bson_build_full (BSON_TYPE_STRING, "title", FALSE, "Post #1", -1,
															BSON_TYPE_UTC_DATETIME, "date", FALSE, 1294860709000,
															BSON_TYPE_ARRAY, "comments", TRUE, bson_build (BSON_TYPE_STRING, "0", "first!", -1,
																				       BSON_TYPE_STRING, "1", "2nd!", -1,
																				       BSON_TYPE_STRING, "2", "last!", -1,
																				       BSON_TYPE_NONE),
															BSON_TYPE_NONE),
									BSON_TYPE_DOCUMENT, "1", TRUE, bson_build (BSON_TYPE_STRING, "title", "Post #2", -1,
														   BSON_TYPE_UTC_DATETIME, "date", 1294860709000,
														   BSON_TYPE_NONE),
									BSON_TYPE_NONE),
		       BSON_TYPE_NONE);
  bson_finish (d);

  o = test_bson_generate_nested ();

  g_assert_cmpint (bson_size (d), ==, bson_size (o));

  g_assert (memcmp (bson_data (d), bson_data (o), bson_size (d)) == 0);

  bson_free (d);
  bson_free (o);

  PASS ();
}

void
test_bson_builder_cursor_as_string (void)
{
  bson *b;
  bson_cursor *c;

  TEST (bson_builder.cursor_as_string);
  b = bson_build (BSON_TYPE_INT32, "i32", 1984,
		  BSON_TYPE_STRING, "s", "Hello World!", -1,
		  BSON_TYPE_NULL, "nil",
		  BSON_TYPE_DOUBLE, "d", 3.14,
		  BSON_TYPE_NONE);
  bson_finish (b);

  g_assert ((c = bson_cursor_new (b)));
  g_assert (bson_cursor_next (c));
  g_assert_cmpstr (bson_cursor_type_as_string (c), ==,
		   "BSON_TYPE_INT32");
  g_assert_cmpint (bson_cursor_type (c), ==,
		   BSON_TYPE_INT32);
  g_assert (bson_cursor_next (c));
  g_assert_cmpstr (bson_cursor_type_as_string (c), ==,
		   "BSON_TYPE_STRING");
  g_assert_cmpint (bson_cursor_type (c), ==,
		   BSON_TYPE_STRING);
  g_assert (bson_cursor_next (c));
  g_assert_cmpstr (bson_cursor_type_as_string (c), ==,
		   "BSON_TYPE_NULL");
  g_assert_cmpint (bson_cursor_type (c), ==,
		   BSON_TYPE_NULL);
  g_assert (bson_cursor_next (c));
  g_assert_cmpstr (bson_cursor_type_as_string (c), ==,
		   "BSON_TYPE_DOUBLE");
  g_assert_cmpint (bson_cursor_type (c), ==,
		   BSON_TYPE_DOUBLE);
  g_assert (!bson_cursor_next (c));

  bson_cursor_free (c);
  bson_free (b);

  PASS ();
}

int
main (void)
{
  PLAN (1, 3);

  test_bson_builder ();
  test_bson_builder_full ();
  test_bson_builder_cursor_as_string ();

  return 0;
}
