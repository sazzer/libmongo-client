#include "test.h"

#include <glib.h>
#include <ctype.h>
#include <stdio.h>

gboolean
dump_bson (bson *b)
{
  gboolean r;

  r = test_bson_dump (b);
  bson_free (b);
  return r;
}

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
	switch (d[i])
	  {
	  case '\\':
	    printf ("\\\\");
	    break;
	  default:
	    printf ("%c", d[i]);
	    break;
	  }
      else
	switch (d[i])
	  {
	  case '\t':
	    printf ("\\t");
	    break;
	  case '\n':
	    printf ("\\n");
	    break;
	  default:
	    printf ("\\x%02x", d[i]);
	    break;
	  }
    }

  printf ("\n");

  return TRUE;
}
