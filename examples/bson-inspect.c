#include "bson.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define _DOC_SIZE(doc,pos) GINT32_FROM_LE (*(gint32 *)(&doc[pos]))

void
bson_dump (bson *b, gint ilevel)
{
  bson_cursor *c;
  gint l;

  c = bson_cursor_new (b);
  while (bson_cursor_next (c))
    {
      for (l = 1; l <= ilevel; l++)
	printf (" ");
      printf ("%s (%s): ",
	      bson_cursor_key (c),
	      bson_cursor_type_as_string (c) + strlen ("BSON_TYPE_"));
      switch (bson_cursor_type (c))
	{
	case BSON_TYPE_DOUBLE:
	  {
	    gdouble d;
	    bson_cursor_get_double (c, &d);
	    printf ("%f\n", d);
	    break;
	  }
	case BSON_TYPE_STRING:
	  {
	    const gchar *s;
	    bson_cursor_get_string (c, &s);
	    printf ("%s\n", s);
	    break;
	  }
	case BSON_TYPE_OID:
	  {
	    const guint8 *oid;
	    gint j;
	    bson_cursor_get_oid (c, &oid);
	    for (j = 0; j < 12; j++)
	      printf ("%x", oid[j]);
	    printf ("\n");
	    break;
	  }
	case BSON_TYPE_BOOLEAN:
	  {
	    gboolean b;
	    bson_cursor_get_boolean (c, &b);
	    printf ((b) ? "TRUE\n" : "FALSE\n");
	    break;
	  }
	case BSON_TYPE_REGEXP:
	  {
	    const gchar *r, *o;
	    bson_cursor_get_regex (c, &r, &o);
	    printf ("/%s/%s\n", r, o);
	    break;
	  }
	case BSON_TYPE_NULL:
	  {
	    printf ("NULL\n");
	    break;
	  }
	case BSON_TYPE_JS_CODE:
	  {
	    const gchar *js;
	    bson_cursor_get_javascript (c, &js);
	    printf ("%s\n", js);
	    break;
	  }
	case BSON_TYPE_SYMBOL:
	  {
	    const gchar *s;
	    bson_cursor_get_symbol (c, &s);
	    printf ("%s\n", s);
	    break;
	  }
	case BSON_TYPE_INT32:
	  {
	    gint32 l32;
	    bson_cursor_get_int32 (c, &l32);
	    printf ("%d\n", l32);
	    break;
	  }
	case BSON_TYPE_INT64:
	  {
	    gint64 l64;
	    bson_cursor_get_int64 (c, &l64);
	    printf ("%ld\n", l64);
	    break;
	  }
	case BSON_TYPE_DOCUMENT:
	  {
	    bson *sd;
	    bson_cursor_get_document (c, &sd);
	    printf ("\n");
	    bson_dump (sd, ilevel + 1);
	    bson_free (sd);
	    break;
	  }
	case BSON_TYPE_ARRAY:
	case BSON_TYPE_JS_CODE_W_SCOPE:
	case BSON_TYPE_BINARY:
	case BSON_TYPE_UNDEFINED:
	case BSON_TYPE_UTC_DATETIME:
	case BSON_TYPE_DBPOINTER:
	case BSON_TYPE_TIMESTAMP:
	case BSON_TYPE_MIN:
	case BSON_TYPE_MAX:
	default:
	  printf ("<unimplemented>\n");
	  break;
	}
    }
  bson_cursor_free (c);
}

int
main (int argc, char *argv[])
{
  int fd;
  off_t offs = 0;
  bson *b;
  guint8 *data;
  struct stat st;
  gint64 i = 1;

  if (argc < 2)
    {
      printf ("Usage: %s FILENAME\n", argv[0]);
      exit (1);
    }

  fd = open (argv[1], O_RDONLY);
  if (fd == -1)
    {
      fprintf (stderr, "Error opening file '%s': %s\n",
	       argv[1], strerror (errno));
      exit (1);
    }
  if (fstat (fd, &st) != 0)
    {
      fprintf (stderr, "Error fstat()ing file '%s': %s\n",
	       argv[1], strerror (errno));
      close (fd);
      exit (1);
    }

  data = mmap (NULL, (size_t)st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED)
    {
      fprintf (stderr, "Error mmap()ing file '%s': %s\n",
	       argv[1], strerror (errno));
      close (fd);
      exit (1);
    }

  while (offs < st.st_size)
    {
      b = bson_new_from_data ((const guint8 *)(data + offs),
			      _DOC_SIZE (data, offs) - 1);
      bson_finish (b);
      offs += bson_size (b);

      printf ("Document #%lu:\n", i);
      bson_dump (b, 1);
      printf ("\n");

      bson_free (b);
      i++;
    }
  munmap (data, st.st_size);
  close (fd);

  return 0;
}
