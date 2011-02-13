#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "bson.h"
#include "test.h"

gboolean
test_bson_empty (void)
{
  bson *b;

  b = bson_new ();
  g_assert (b);
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_string (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_string (b, "hello", "world", -1));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_string_len (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_string
	    (b, "goodbye", "cruel world, this garbage is gone.",
	     strlen ("cruel world")));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_double (void)
{
  bson *b;
  double d = 3.14;

  b = bson_new ();
  g_assert (bson_append_double (b, "double", d));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_boolean (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_boolean (b, "FALSE", FALSE));
  g_assert (bson_append_boolean (b, "TRUE", TRUE));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_utc_datetime (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_utc_datetime (b, "date", 1294860709000));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_null (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_null (b, "null"));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_int32 (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_int32 (b, "int32", 1984));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_int64 (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_int64 (b, "int64", 9876543210));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_regexp (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_regex (b, "regexp", "foo.*bar", "i"));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_binary0 (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_binary (b, "binary0", BSON_BINARY_SUBTYPE_GENERIC,
				7, (guint8 *)"foo\0bar"));
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_binary2 (void)
{
  bson *b;

  b = bson_new ();
  g_assert (bson_append_binary (b, "binary2", BSON_BINARY_SUBTYPE_BINARY,
				11, (guint8 *)"\0\0\0\7foo\0bar"));
  bson_finish (b);

  return dump_bson (b);
}

int
main (void)
{
  g_assert (test_bson_empty ());
  g_assert (test_bson_string ());
  g_assert (test_bson_string_len ());
  g_assert (test_bson_double ());
  g_assert (test_bson_boolean ());
  g_assert (test_bson_utc_datetime ());
  g_assert (test_bson_null ());
  g_assert (test_bson_int32 ());
  g_assert (test_bson_int64 ());
  g_assert (test_bson_regexp ());
  g_assert (test_bson_binary0 ());
  g_assert (test_bson_binary2 ());

  return 0;
}
