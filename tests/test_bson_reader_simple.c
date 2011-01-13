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
test_bson_reader_simple (void)
{
  bson *b;
  bson_cursor *c;

  gint32 i;
  gboolean bool;
  const gchar *s;
  gint64 l;
  gdouble d;

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

  TEST (reader_find_string);
  c = bson_find (b, "hello");
  ENSURE_TYPE (b, c, BSON_TYPE_STRING);
  ENSURE_VALUE (b, c, string, &s);
  if (strcmp (s, "world"))
    BAIL_OUT (b, c);
  g_free (c);
  PASS ();

  TEST (reder_find_boolean_true);
  c = bson_find (b, "TRUE");
  ENSURE_TYPE (b, c, BSON_TYPE_BOOLEAN);
  ENSURE_VALUE (b, c, boolean, &bool);
  if (bool != TRUE)
    BAIL_OUT (b, c);
  g_free (c);
  PASS ();

  TEST (reader_find_null);
  c = bson_find (b, "null");
  ENSURE_TYPE (b, c, BSON_TYPE_NULL);
  g_free (c);
  PASS ();

  TEST (reader_find_int32);
  c = bson_find (b, "int32");
  ENSURE_TYPE (b, c, BSON_TYPE_INT32);
  ENSURE_VALUE (b, c, int32, &i);
  if (i != 1984)
    BAIL_OUT (b, c);
  g_free (c);
  PASS ();

  TEST (reader_find_boolean_false);
  c = bson_find (b, "FALSE");
  ENSURE_TYPE (b, c, BSON_TYPE_BOOLEAN);
  ENSURE_VALUE (b, c, boolean, &bool);
  if (bool != FALSE)
    BAIL_OUT (b, c);
  g_free (c);
  PASS ();

  TEST (reader_find_string_2);
  c = bson_find (b, "goodbye");
  ENSURE_TYPE (b, c, BSON_TYPE_STRING);
  ENSURE_VALUE (b, c, string, &s);
  if (strcmp (s, "cruel world"))
    BAIL_OUT (b, c);
  g_free (c);
  PASS ();

  TEST (reader_find_date);
  c = bson_find (b, "date");
  ENSURE_TYPE (b, c, BSON_TYPE_UTC_DATETIME);
  ENSURE_VALUE (b, c, utc_datetime, &l);
  if (l != 1294860709000)
    BAIL_OUT (b, c);
  g_free (c);
  PASS ();

  TEST (reader_find_double);
  c = bson_find (b, "double");
  ENSURE_TYPE (b, c, BSON_TYPE_DOUBLE);
  ENSURE_VALUE (b, c, double, &d);
  if (d != 3.14)
    BAIL_OUT (b, c);
  g_free (c);
  PASS ();

  TEST (reader_find_int64);
  c = bson_find (b, "int64");
  ENSURE_TYPE (b, c, BSON_TYPE_INT64);
  ENSURE_VALUE (b, c, int64, &l);
  if (l != 9876543210)
    BAIL_OUT (b, c);
  g_free (c);
  PASS ();

  bson_free (b);

  return TRUE;
}

int
main (void)
{
  return !test_bson_reader_simple ();
}
