#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "bson.h"
#include "test.h"
#include "test-generator.h"

static gboolean
test_bson_reader_flat (void)
{
  bson *b;
  bson_cursor *c;

  gint32 i;
  gboolean bool;
  const gchar *s;
  gint64 l;
  gdouble d;

  b = test_bson_generate_flat ();

  TEST (bson_reader_find_string);
  g_assert ((c = bson_find (b, "hello")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_assert (bson_cursor_get_string (c, &s));
  g_assert_cmpstr (s, ==, "world");
  g_free (c);
  PASS ();

  TEST (bson_reder_find_boolean_true);
  g_assert ((c = bson_find (b, "TRUE")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BOOLEAN);
  g_assert (bson_cursor_get_boolean (c, &bool));
  g_assert (bool == TRUE);
  g_free (c);
  PASS ();

  TEST (bson_reader_find_null);
  g_assert ((c = bson_find (b, "null")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_NULL);
  g_free (c);
  PASS ();

  TEST (bson_reader_find_int32);
  g_assert ((c = bson_find (b, "int32")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_INT32);
  g_assert (bson_cursor_get_int32 (c, &i));
  g_assert_cmpint (i, ==, 1984);
  g_free (c);
  PASS ();

  TEST (bson_reader_find_boolean_false);
  g_assert ((c = bson_find (b, "FALSE")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BOOLEAN);
  g_assert (bson_cursor_get_boolean (c, &bool));
  g_assert (bool == FALSE);
  g_free (c);
  PASS ();

  TEST (bson_reader_find_string_2);
  g_assert ((c = bson_find (b, "goodbye")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_assert (bson_cursor_get_string (c, &s));
  g_assert_cmpstr (s, ==, "cruel world");
  g_free (c);
  PASS ();

  TEST (bson_reader_find_date);
  g_assert ((c = bson_find (b, "date")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_UTC_DATETIME);
  g_assert (bson_cursor_get_utc_datetime (c, &l));
  g_assert_cmpint (l, ==, 1294860709000);
  g_free (c);
  PASS ();

  TEST (bson_reader_find_double);
  g_assert ((c = bson_find (b, "double")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOUBLE);
  g_assert (bson_cursor_get_double (c, &d));
  g_assert_cmpfloat (d, ==, 3.14);
  g_free (c);
  PASS ();

  TEST (bson_reader_find_int64);
  g_assert ((c = bson_find (b, "int64")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_INT64);
  g_assert (bson_cursor_get_int64 (c, &l));
  g_assert_cmpint (l, ==, 9876543210);
  g_free (c);
  PASS ();

  bson_free (b);

  return TRUE;
}

static gboolean
test_bson_reader_nested (void)
{
  bson *b, *t;
  bson_cursor *c;

  b = test_bson_generate_nested ();

  TEST (bson_reader_complex_find_user);
  g_assert ((c = bson_find (b, "user")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOCUMENT);
  g_assert (bson_cursor_get_document (c, &t));
  PASS ();

  {
    const char *s;
    gint32 i;

    bson_cursor *c2;

    TEST (bson_reader_complex_find_user.name);
    g_assert ((c2 = bson_find (t, "name")));
    g_assert_cmpint (bson_cursor_type (c2), ==, BSON_TYPE_STRING);
    g_assert (bson_cursor_get_string (c2, &s));
    g_assert_cmpstr (s, ==, "V.A. Lucky");
    g_free (c2);
    PASS ();

    TEST (bson_reader_complex_find_user.id);
    g_assert ((c2 = bson_find (t, "id")));
    g_assert_cmpint (bson_cursor_type (c2), ==, BSON_TYPE_INT32);
    g_assert (bson_cursor_get_int32 (c2, &i));
    g_assert_cmpint (i, ==, 12345);
    g_free (c2);
    PASS ();
  }

  g_free (c);
  bson_free (t);

  TEST (bson_reader_complex_find_posts);
  g_assert ((c = bson_find (b, "posts")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
  g_assert (bson_cursor_get_array (c, &t));
  PASS ();

  {
    bson_cursor *c2;
    bson *d;

    TEST (bson_reader_complex_find_posts.1);
    g_assert ((c2 = bson_find (t, "1")));
    g_assert_cmpint (bson_cursor_type (c2), ==, BSON_TYPE_DOCUMENT);
    g_assert (bson_cursor_get_document (c2, &d));
    PASS ();

    {
      bson_cursor *c3;

      TEST (bson_reader_complex_find_posts.1.comments);
      g_assert ((c3 = bson_find (d, "comments")) == NULL);
      PASS ();
    }

    g_free (c2);
    bson_free (d);

    TEST (bson_reader_complex_find_posts.0);
    g_assert ((c2 = bson_find (t, "0")));
    g_assert_cmpint (bson_cursor_type (c2), ==, BSON_TYPE_DOCUMENT);
    g_assert (bson_cursor_get_document (c2, &d));
    PASS ();

    {
      bson_cursor *c3;
      bson *a;

      TEST (bson_reader_complex_find_posts.0.comments);
      g_assert ((c3 = bson_find (d, "comments")));
      g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
      g_assert (bson_cursor_get_array (c3, &a));
      PASS ();

      {
	bson_cursor *c4;
	const gchar *s;

	TEST (bson_reader_complex_find_posts.0.comments.2);
	g_assert ((c4 = bson_find (a, "2")));
	g_assert_cmpint (bson_cursor_type (c4), ==, BSON_TYPE_STRING);
	g_assert (bson_cursor_get_string (c4, &s));
	g_assert_cmpstr (s, ==, "last!");
	g_free (c4);
	PASS ();
      }

      g_free (c3);
      bson_free (a);
    }

    g_free (c2);
    bson_free (d);
  }

  g_free (c);
  bson_free (b);

  return TRUE;
}

static gboolean
test_bson_reader_regexp (void)
{
  bson *b;
  bson_cursor *c;
  const gchar *regexp, *flags, *str;

  b = bson_new ();
  g_assert (bson_append_string (b, "string1", "test1", -1));
  g_assert (bson_append_regex (b, "regexp", "foo.*bar", "i"));
  g_assert (bson_append_string (b, "string2", "test2", -1));
  bson_finish (b);

  TEST(bson_reader_regexp.seek);
  g_assert ((c = bson_find (b, "string2")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_assert (bson_cursor_get_string (c, &str));
  g_assert_cmpstr (str, ==, "test2");
  g_free (c);
  PASS ();

  TEST(bson_reader_regexp.read);
  g_assert ((c = bson_find (b, "regexp")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_REGEXP);
  g_assert (bson_cursor_get_regex (c, &regexp, &flags));
  g_assert_cmpstr (regexp, ==, "foo.*bar");
  g_assert_cmpstr (flags, ==, "i");
  g_free (c);
  PASS ();

  return TRUE;
}

static gboolean
test_bson_reader_binary (void)
{
  bson *b;
  bson_cursor *c;
  const guint8 *binary;
  gint32 size;
  bson_binary_subtype subtype;

  b = bson_new ();
  g_assert (bson_append_binary (b, "binary0", BSON_BINARY_SUBTYPE_GENERIC,
				(guint8 *)"foo\0bar", 7));
  g_assert (bson_append_binary (b, "binary2", BSON_BINARY_SUBTYPE_BINARY,
				(guint8 *)"\0\0\0\7foo\0bar", 11));
  bson_finish (b);

  TEST(bson_reader_binary.0);
  g_assert ((c = bson_find (b, "binary0")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BINARY);
  g_assert (bson_cursor_get_binary (c, &subtype, &binary, &size));
  g_assert_cmpint (subtype, ==, BSON_BINARY_SUBTYPE_GENERIC);
  g_assert_cmpint (size, ==, 7);
  g_assert (!memcmp (binary, "foo\0bar", 7));
  g_free (c);
  PASS ();

  TEST(bson_reader_binary.2);
  g_assert ((c = bson_find (b, "binary2")));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BINARY);
  g_assert (bson_cursor_get_binary (c, &subtype, &binary, &size));
  g_assert_cmpint (subtype, ==, BSON_BINARY_SUBTYPE_BINARY);
  g_assert_cmpint (size, ==, 11);
  g_assert (!memcmp (binary, "\0\0\0\7foo\0bar", 11));
  g_free (c);
  PASS ();

  return TRUE;
}

static gboolean
test_bson_reader_huge (void)
{
  bson *b, *s;
  bson_cursor *c;
  gchar buffer[32768];
  gint32 ds1, ds2;

  TEST(bson_reader_huge);

  memset (buffer, 'a', sizeof (buffer));
  buffer[sizeof(buffer) - 1] = '\0';
  b = bson_new ();
  g_assert (bson_append_int32 (b, "preamble", 1));
  g_assert (bson_append_string (b, "huge", buffer, -1));
  g_assert (bson_append_int32 (b, "post", 1234));
  bson_finish (b);

  ds1 = bson_size (b);

  s = bson_new ();
  g_assert (bson_append_document (s, "hugedoc", b));
  bson_finish (s);
  bson_free (b);

  g_assert_cmpint (bson_size (s), >, ds1);

  c = bson_find (s, "hugedoc");
  bson_cursor_get_document (c, &b);

  ds2 = bson_size (b);
  g_assert_cmpint (ds1, ==, ds2);

  g_free (c);
  bson_free (b);
  bson_free (s);
  PASS();

  return TRUE;
}

int
main (void)
{
  PLAN (1, 23);

  g_assert (test_bson_reader_flat ());
  g_assert (test_bson_reader_nested ());
  g_assert (test_bson_reader_regexp ());
  g_assert (test_bson_reader_binary ());
  g_assert (test_bson_reader_huge ());
}
