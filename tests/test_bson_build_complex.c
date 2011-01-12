#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "bson.h"
#include "test.h"

gboolean
test_bson_complex_1 (void)
{
  bson *b;

  b = bson_new ();
  if (!bson_append_int32 (b, "int32", 1984))
    return FALSE;
  if (!bson_append_boolean (b, "FALSE", FALSE))
    return FALSE;
  if (!bson_append_string (b, "goodbye", "cruel world, this garbage is gone.",
			   strlen ("cruel world")))
    return FALSE;
  if (!bson_append_utc_datetime (b, "date", 1294860709000))
    return FALSE;
  if (!bson_append_double (b, "double", 3.14))
    return FALSE;
  if (!bson_append_int64 (b, "int64", 9876543210))
    return FALSE;
  if (!bson_append_null (b, "null"))
    return FALSE;
  if (!bson_append_boolean (b, "TRUE", TRUE))
    return FALSE;
  if (!bson_append_string (b, "hello", "world", -1))
    return FALSE;

  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_complex_2 (void)
{
  bson *d, *user, *posts;
  bson *p1, *p2, *comments;

  user = bson_new ();
  bson_append_string (user, "name", "V.A. Lucky", -1);
  bson_append_int32 (user, "id", 12345);
  bson_finish (user);

  comments = bson_new ();
  bson_append_string (comments, "0", "first!", -1);
  bson_append_string (comments, "1", "2nd!", -1);
  bson_append_string (comments, "2", "last!", -1);
  bson_finish (comments);

  p1 = bson_new ();
  bson_append_utc_datetime (p1, "date", 1294860709000);
  bson_append_array (p1, "comments", comments);
  bson_append_string (p1, "title", "Post #1", -1);
  bson_finish (p1);
  bson_free (comments);

  p2 = bson_new ();
  bson_append_utc_datetime (p2, "date", 1294860709000);
  bson_append_string (p2, "title", "Post #2", -1);
  bson_finish (p2);

  posts = bson_new ();
  bson_append_document (posts, "0", p1);
  bson_append_document (posts, "1", p2);
  bson_finish (posts);
  bson_free (p1);
  bson_free (p2);

  d = bson_new ();
  bson_append_array (d, "posts", posts);
  bson_append_document (d, "user", user);
  bson_finish (d);

  bson_free (user);
  bson_free (posts);

  return dump_bson (d);
}

int
main (void)
{
  if (!test_bson_complex_1 ())
    return -1;

  if (!test_bson_complex_2 ())
    return -1;

  return 0;
}
