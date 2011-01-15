#include "test.h"

#include <glib.h>
#include <ctype.h>
#include <stdio.h>

gboolean
dump_bson (bson *b)
{
  gboolean r;

  r = dump_data (bson_data (b), bson_size (b));
  bson_free (b);
  return r;
}

gboolean
dump_data (const guint8 *d, gint32 size)
{
  gint32 i;

  if (!d || size <= 0)
    return FALSE;

  for (i = 0; i < size; i++)
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
