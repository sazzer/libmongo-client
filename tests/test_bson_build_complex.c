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

int
main (void)
{
  if (!test_bson_complex_1 ())
    return -1;

  return 0;
}
