#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "bson.h"
#include "test.h"

static gboolean
dump_bson (bson *b)
{
  gboolean r;

  r = test_bson_dump (b);
  bson_free (b);
  return r;
}

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

  return 0;
}
