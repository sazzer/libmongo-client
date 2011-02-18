#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <glib.h>

#include "bson.h"
#include "test.h"
#include "test-generator.h"

static gboolean
test_bson_reader_iterate_flat (void)
{
  bson *b;
  bson_cursor *c;

  gint32 i;
  gboolean bool;
  const gchar *s;
  gint64 l;
  gdouble d;

  b = test_bson_generate_flat ();

  c = bson_cursor_new (b);

  TEST (bson_reader_iterate.1);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_INT32);
  g_assert (bson_cursor_get_int32 (c, &i));
  g_assert_cmpint (i, ==, 1984);
  PASS ();

  TEST (bson_reader_iterate.2);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BOOLEAN);
  g_assert (bson_cursor_get_boolean (c, &bool));
  g_assert (bool == FALSE);
  PASS ();

  TEST (bson_reader_iterate.3);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_assert (bson_cursor_get_string (c, &s));
  g_assert_cmpstr (s, ==, "cruel world");
  PASS ();

  TEST (bson_reader_iterate.4);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_UTC_DATETIME);
  g_assert (bson_cursor_get_utc_datetime (c, &l));
  g_assert_cmpint (l, ==, 1294860709000);
  PASS ();

  TEST (bson_reader_iterate.5);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOUBLE);
  g_assert (bson_cursor_get_double (c, &d));
  g_assert_cmpfloat (d, ==, 3.14);
  PASS ();

  TEST (bson_reader_iterate.6);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_INT64);
  g_assert (bson_cursor_get_int64 (c, &l));
  g_assert_cmpint (l, ==, 9876543210);
  PASS ();

  TEST (bson_reader_iterate.7);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_NULL);
  PASS ();

  TEST (bson_reader_iterate.8);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_BOOLEAN);
  g_assert (bson_cursor_get_boolean (c, &bool));
  g_assert (bool == TRUE);
  PASS ();

  TEST (bson_reader_iterate.9);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_STRING);
  g_assert (bson_cursor_get_string (c, &s));
  g_assert_cmpstr (s, ==, "world");
  PASS ();

  TEST (bson_reader_iterate.10);
  g_assert (!bson_cursor_next (c));
  PASS ();

  bson_cursor_free (c);
  bson_free (b);

  return TRUE;
}

static gboolean
test_bson_reader_iterate_nested (void)
{
  bson *b;
  bson_cursor *c;

  b = test_bson_generate_nested ();

  c = bson_cursor_new (b);

  TEST (bson_reader_nested_iterate.1);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_DOCUMENT);
  PASS ();

  TEST (bson_reader_nested_iterate.2);
  g_assert (bson_cursor_next (c));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_ARRAY);
  PASS ();

  TEST (bson_reader_nested_iterate.3);
  g_assert (!bson_cursor_next (c));
  PASS ();

  bson_cursor_free (c);
  bson_free (b);

  return TRUE;
}

int
main (void)
{
  PLAN (1, 13);

  g_assert (test_bson_reader_iterate_flat ());
  g_assert (test_bson_reader_iterate_nested ());
}
