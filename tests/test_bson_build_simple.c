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
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_string (void)
{
  bson *b;

  b = bson_new ();
  if (!bson_append_string (b, "hello", "world", -1))
    return FALSE;
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_string_len (void)
{
  bson *b;

  b = bson_new ();
  if (!bson_append_string (b, "goodbye", "cruel world, this garbage is gone.",
			   strlen ("cruel world")))
    return FALSE;
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_double (void)
{
  bson *b;
  double d = 3.14;

  b = bson_new ();
  if (!bson_append_double (b, "double", d))
    return FALSE;
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_boolean (void)
{
  bson *b;

  b = bson_new ();
  if (!bson_append_boolean (b, "FALSE", FALSE))
    return FALSE;
  if (!bson_append_boolean (b, "TRUE", TRUE))
    return FALSE;
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_utc_datetime (void)
{
  bson *b;

  b = bson_new ();
  if (!bson_append_utc_datetime (b, "date", 1294860709000))
    return FALSE;
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_null (void)
{
  bson *b;

  b = bson_new ();
  if (!bson_append_null (b, "null"))
    return FALSE;
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_int32 (void)
{
  bson *b;

  b = bson_new ();
  if (!bson_append_int32 (b, "int32", 1984))
    return FALSE;
  bson_finish (b);

  return dump_bson (b);
}

gboolean
test_bson_int64 (void)
{
  bson *b;

  b = bson_new ();
  if (!bson_append_int64 (b, "int64", 9876543210))
    return FALSE;
  bson_finish (b);

  return dump_bson (b);
}

int
main (void)
{
  if (!test_bson_empty ())
    return -1;

  if (!test_bson_string ())
    return -1;

  if (!test_bson_string_len ())
    return -1;

  if (!test_bson_double ())
    return -1;

  if (!test_bson_boolean ())
    return -1;

  if (!test_bson_utc_datetime ())
    return -1;

  if (!test_bson_null ())
    return -1;

  if (!test_bson_int32 ())
    return -1;

  if (!test_bson_int64 ())
    return -1;

  return 0;
}
