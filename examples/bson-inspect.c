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

#include <glib.h>

#define _DOC_SIZE(doc,pos) GINT32_FROM_LE (*(gint32 *)(&doc[pos]))

static void
_indent (gint level, gboolean verbose)
{
  gint i;

  if (!verbose)
    return;

  for (i = 1; i <= level; i++)
    printf ("  ");
}

static void
bson_dump (bson *b, gint ilevel, gboolean verbose, gboolean as_array)
{
  bson_cursor *c;
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
	{
	  _indent (ilevel, verbose);
	  printf ("/* type='%s'; */\n",
		  bson_cursor_type_as_string (c) + 10);
	}
      _indent (ilevel, verbose);
      if (!as_array)
	{
	  printf ("\"%s\" : ", bson_cursor_key (c));
	}
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
	    printf ("%" G_GINT64_FORMAT, l64);
	    break;
	  }
	case BSON_TYPE_DOCUMENT:
	  {
	    bson *sd;
	    bson_cursor_get_document (c, &sd);
	    printf ("{ ");
	    if (verbose)
	      printf ("/* size='%d' */\n", bson_size (sd));
	    bson_dump (sd, ilevel + 1, verbose, FALSE);
	    if (verbose)
	      {
		printf ("\n");
		_indent (ilevel, verbose);
		printf ("}");
	      }
	    else
	      printf (" }");
	    bson_free (sd);
	    break;
	  }
	case BSON_TYPE_ARRAY:
	  {
	    bson *sa;

	    bson_cursor_get_array (c, &sa);
	    printf ("[ ");
	    if (verbose)
	      printf ("/* size='%d' */\n", bson_size (sa));
	    bson_dump (sa, ilevel + 1, verbose, TRUE);
	    if (verbose)
	      {
		printf ("\n");
		_indent (ilevel, verbose);
		printf ("]");
	      }
	    else
	      printf (" ]");
	    bson_free (sa);
	    break;
	  }
	case BSON_TYPE_BINARY:
	  {
	    const guint8 *data;
	    gint32 size;
	    bson_binary_subtype t;
	    gchar *b64;

	    bson_cursor_get_binary (c, &t, &data, &size);
	    b64 = g_base64_encode (data, size);
	    printf ("{ ");
	    if (verbose)
	      {
		printf ("/* size='%d' */\n", size);
		_indent (ilevel + 1, verbose);
	      }
	    printf ("\"$binary\" : \"%s\",", b64);
	    if (verbose)
	      {
		printf ("\n");
		_indent (ilevel + 1, verbose);
	      }
	    else
	      printf (" ");
	    printf ("\"$type\" : \"%02d\"", t);
	    if (verbose)
	      {
		printf ("\n");
		_indent (ilevel, verbose);
	      }
	    else
	      printf (" ");
	    printf ("}");
	    g_free (b64);
	    break;
	  }
	case BSON_TYPE_JS_CODE_W_SCOPE:
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
  GOptionContext *context;
  gboolean verbose = FALSE;
  GError *error = NULL;

  GOptionEntry entries[] =
    {
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
	"Be verbose", NULL },
      { NULL }
    };

  context = g_option_context_new ("- inspect a BSON dump");
  g_option_context_add_main_entries (context, entries, "bson-inspect");
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("option parsing failed: %s\n", error->message);
      exit (1);
    }

  if (argc < 2)
    {
      gchar *help = g_option_context_get_help (context, TRUE, NULL);

      printf ("%s", help);
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

      if (verbose)
	printf ("/* Document #%" G_GUINT64_FORMAT "; size='%d' */\n", i,
		bson_size (b));
      printf ("{ ");
      if (verbose)
	printf ("\n");
      bson_dump (b, 1, verbose, FALSE);
      if (verbose)
	printf ("\n}\n");
      else
	printf (" }\n");
      if (verbose)
	printf ("\n");

      bson_free (b);
      i++;
    }
  munmap (data, st.st_size);
  close (fd);

  return 0;
}
