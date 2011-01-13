#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "bson.h"
#include "test.h"

static gchar *current_test = NULL;

#define BAIL_OUT(b,c)				\
  {						\
    FAIL ();					\
    bson_free (b);				\
    g_free (c);					\
    return FALSE;				\
  }

#define ENSURE_TYPE(b,c,t)						\
  if (bson_cursor_type (c) != t)					\
    {									\
      printf ("# Element `%s' does not match type %s\n",		\
	      bson_cursor_key (c), #t);					\
      BAIL_OUT (b,c);							\
    }

#define ENSURE_VALUE(b,c,t,p)			\
  if (!bson_cursor_get_ ## t (c, p))		\
    BAIL_OUT (b, c);				\

#define TEST(s) current_test = #s
#define PASS() printf ("PASS: %s\n", current_test)
#define FAIL() printf ("FAIL: %s\n", current_test)

static gboolean
test_bson_reader_complex (void)
{
  bson *b, *user, *posts;
  bson *p1, *p2, *comments;
  bson_cursor *c1, *c2, *c3, *c4;

  const gchar *s;
  gint32 i;

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
  bson_append_string (p1, "title", "Post #1", -1);
  bson_append_utc_datetime (p1, "date", 1294860709000);
  bson_append_array (p1, "comments", comments);
  bson_finish (p1);
  bson_free (comments);

  p2 = bson_new ();
  bson_append_string (p2, "title", "Post #2", -1);
  bson_append_utc_datetime (p2, "date", 1294860709000);
  bson_finish (p2);

  posts = bson_new ();
  bson_append_document (posts, "0", p1);
  bson_append_document (posts, "1", p2);
  bson_finish (posts);
  bson_free (p1);
  bson_free (p2);

  b = bson_new ();
  bson_append_document (b, "user", user);
  bson_append_array (b, "posts", posts);
  bson_finish (b);

  bson_free (user);
  bson_free (posts);

  TEST (reader_complex_find_user);
  c1 = bson_find (b, "user");
  ENSURE_TYPE (b, c1, BSON_TYPE_DOCUMENT);
  ENSURE_VALUE (b, c1, document, &user);
  PASS ();

  TEST (reader_complex_find_user.name);
  c2 = bson_find (user, "name");
  ENSURE_TYPE (b, c2, BSON_TYPE_STRING);
  ENSURE_VALUE (b, c2, string, &s);
  if (strcmp (s, "V.A. Lucky"))
    BAIL_OUT (b, c1);
  g_free (c2);
  PASS ();

  TEST (reader_complex_find_user.id);
  c2 = bson_find (user, "id");
  ENSURE_TYPE (b, c2, BSON_TYPE_INT32);
  ENSURE_VALUE (b, c2, int32, &i);
  if (i != 12345)
    BAIL_OUT (b, c1);
  g_free (c2);
  g_free (c1);
  bson_free (user);
  PASS ();

  TEST (reader_complex_find_posts);
  c1 = bson_find (b, "posts");
  ENSURE_TYPE (b, c1, BSON_TYPE_ARRAY);
  ENSURE_VALUE (b, c1, array, &posts);
  PASS ();

  TEST (reader_complex_find_posts.1);
  c2 = bson_find (posts, "1");
  ENSURE_TYPE (b, c2, BSON_TYPE_DOCUMENT);
  ENSURE_VALUE (b, c2, document, &p1);
  PASS ();

  TEST (reader_complex_find_posts.1.comments);
  c3 = bson_find (p1, "comments");
  if (c3)
    BAIL_OUT (b, c1);
  PASS ();

  TEST (reader_complex_find_posts.0);
  c2 = bson_find (posts, "0");
  ENSURE_TYPE (b, c2, BSON_TYPE_DOCUMENT);
  ENSURE_VALUE (b, c2, document, &p1);
  PASS ();

  TEST (reader_complex_find_posts.0.comments);
  c3 = bson_find (p1, "comments");
  ENSURE_TYPE (b, c3, BSON_TYPE_ARRAY);
  ENSURE_VALUE (b, c3, array, &comments);
  PASS ();

  TEST (reader_complex_find_posts.0.comments.2);
  c4 = bson_find (comments, "2");
  ENSURE_TYPE (b, c4, BSON_TYPE_STRING);
  ENSURE_VALUE (b, c4, string, &s);
  if (strcmp (s, "last!"))
    BAIL_OUT (b, c1);
  PASS ();

  return TRUE;
}

int
main (void)
{
  return !test_bson_reader_complex ();
}
