#include "test.h"

#include <glib.h>
#include <ctype.h>
#include <stdio.h>

gboolean
test_bson_dump (bson *b)
{
  gint32 i;
  const guint8 *d;

  d = bson_data (b);
  if (!d)
    return FALSE;

  for (i = 0; i < bson_size (b); i++)
    {
      if (isprint (d[i]))
	printf ("%c", d[i]);
      else
	switch (d[i])
	  {
	  case '\t':
	    printf ("\\t");
	    break;
	  default:
	    printf ("\\x%02x", d[i]);
	    break;
	  }
    }

  printf ("\n");

  return TRUE;
}
