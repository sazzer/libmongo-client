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
bson_dump (bson *b, gint ilevel, gboolean verbose)
{
  bson_cursor *c;
  gint l;
  gboolean first = TRUE;

  c = bson_cursor_new (b);
  while (bson_cursor_next (c))
    {
      if (!first)
	{
	  printf (", ");
	  if (verbose)
	    printf ("\n");
	}
      first = FALSE;
      if (verbose)
	for (l = 1; l <= ilevel; l++)
	  printf (" ");
      printf ("\"%s\" : ", bson_cursor_key (c));
      switch (bson_cursor_type (c))
	{
	case BSON_TYPE_DOUBLE:
	  {
	    gdouble d;
	    bson_cursor_get_double (c, &d);
	    printf ("%f", d);
	    break;
	  }
	case BSON_TYPE_STRING:
	  {
	    const gchar *s;
	    gchar *s2;
	    bson_cursor_get_string (c, &s);
	    s2 = g_strescape (s, NULL);
	    printf ("\"%s\"", s2);
	    g_free (s2);
	    break;
	  }
	case BSON_TYPE_OID:
	  {
	    const guint8 *oid;
	    gint j;
	    bson_cursor_get_oid (c, &oid);
	    printf ("ObjectId( \"");
	    for (j = 0; j < 12; j++)
	      printf ("%02x", oid[j]);
	    printf ("\" )");
	    break;
	  }
	case BSON_TYPE_BOOLEAN:
	  {
	    gboolean b;
	    bson_cursor_get_boolean (c, &b);
	    printf ((b) ? "true" : "false");
	    break;
	  }
	case BSON_TYPE_REGEXP:
	  {
	    const gchar *r, *o;
	    gchar *r2, *o2;
	    bson_cursor_get_regex (c, &r, &o);
	    r2 = g_strescape (r, NULL);
	    o2 = g_strescape (o, NULL);
	    printf ("Regex(\"/%s/%s\")", r2, o2);
	    g_free (r2);
	    g_free (o2);
	    break;
	  }
	case BSON_TYPE_NULL:
	  {
	    printf ("null");
	    break;
	  }
	case BSON_TYPE_JS_CODE:
	  {
	    const gchar *js;
	    gchar *js2;
	    bson_cursor_get_javascript (c, &js);
	    js2 = g_strescape (js, NULL);
	    printf ("%s", js2);
	    g_free (js2);
	    break;
	  }
	case BSON_TYPE_SYMBOL:
	  {
	    const gchar *s;
	    gchar *s2;
	    bson_cursor_get_symbol (c, &s);
	    s2 = g_strescape (s, NULL);
	    printf ("%s", s2);
	    g_free (s2);
	    break;
	  }
	case BSON_TYPE_INT32:
	  {
	    gint32 l32;
	    bson_cursor_get_int32 (c, &l32);
	    printf ("%d", l32);
	    break;
	  }
	case BSON_TYPE_INT64:
	  {
	    gint64 l64;
	    bson_cursor_get_int64 (c, &l64);
	    printf ("%ld", l64);
	    break;
	  }
	case BSON_TYPE_DOCUMENT:
	  {
	    bson *sd;
	    bson_cursor_get_document (c, &sd);
	    printf ("{ ");
	    if (verbose)
	      printf ("\n");
	    bson_dump (sd, ilevel + 1, FALSE);
	    if (verbose)
	      for (l = 1; l <= ilevel; l++)
		printf (" ");
	    printf (" }");
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
	  printf ("\"<unimplemented>\"");
	  break;
	}
      if (verbose)
	printf ("/* %s */", bson_cursor_type_as_string (c) + 10);
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

      //printf ("/* Document #%lu */\n", i);
      printf ("{ ");
      //printf ("\n");
      bson_dump (b, 1, FALSE);
      printf (" }\n");
      //printf ("\n");

      bson_free (b);
      i++;
    }
  munmap (data, st.st_size);
  close (fd);

  return 0;
}
