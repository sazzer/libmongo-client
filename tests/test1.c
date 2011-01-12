#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "bson.h"

int
main (void)
{
  bson *b;
  gint32 i;

  bson *a;

  const guint8 *d;

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

  for (i = 0; i < bson_size (b); i++)
    {
      if (isprint (d[i]))
	printf ("%c", d[i]);
      else
	printf ("\\x%02x", d[i]);
    }

  printf ("\n");
  bson_free (b);
  
  return 0;
}

