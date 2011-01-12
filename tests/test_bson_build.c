#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "bson.h"
#include "test.h"

int
main (void)
{
  bson *b;
  gint32 i;

  bson *a;

  const guint8 *d;

  gboolean r;

  a = bson_new ();
  bson_append_string (a, "0", "awesome", -1);
  bson_append_double (a, "1", 5.05);
  bson_append_int32 (a, "2", 1986);
  bson_finish (a);

  b = bson_new ();
  bson_append_array (b, "BSON", a);
  bson_finish (b);

  bson_free (a);
  
  d = bson_data (b);

  r = test_bson_dump (b);
  bson_free (b);

  return (r) ? 0 : -1;
}
