#include "bson.h"
#include "tap.h"
#include "test.h"

#include "libmongo-private.h"

#include <string.h>

static void
test_func_weird_types (void)
{
  bson *b;
  bson_cursor *c;
  guint8 type = BSON_TYPE_DBPOINTER;
  gint32 slen;

  b = bson_new ();
  bson_append_int32 (b, "int32", 42);

  /* Append weird stuff */
  b->data = g_byte_array_append (b->data, (const guint8 *)&type, sizeof (type));
  b->data = g_byte_array_append (b->data, (const guint8 *)"dbpointer",
				 strlen ("dbpointer") + 1);
  slen = GINT32_TO_LE (strlen ("refname") + 1);
  b->data = g_byte_array_append (b->data, (const guint8 *)&slen, sizeof (gint32));
  b->data = g_byte_array_append (b->data, (const guint8 *)"refname",
				 strlen ("refname") + 1);
  b->data = g_byte_array_append (b->data, (const guint8 *)"0123456789ABCDEF",
				 12);

  bson_append_boolean (b, "Here be dragons?", TRUE);
  bson_finish (b);

  c = bson_find (b, "Here be dragons?");
  ok (c != NULL,
      "bson_find() can find elements past unsupported BSON types");
  bson_cursor_free (c);
  bson_free (b);

  /* Now do it again, but append a type we can't iterate over */
  b = bson_new ();
  bson_append_int32 (b, "int32", 42);

  /* Append BSON_TYPE_NONE */
  type = BSON_TYPE_NONE;
  b->data = g_byte_array_append (b->data, (const guint8 *)&type, sizeof (type));
  b->data = g_byte_array_append (b->data, (const guint8 *)"dbpointer",
				 strlen ("dbpointer") + 1);
  b->data = g_byte_array_append (b->data, (const guint8 *)"0123456789ABCDEF",
				 12);

  bson_append_boolean (b, "Here be dragons?", TRUE);
  bson_finish (b);

  c = bson_find (b, "Here be dragons?");
  ok (c == NULL,
      "bson_find() should bail out when encountering an invalid element.");
  bson_cursor_free (c);

  c = bson_cursor_new (b);
  bson_cursor_next (c); /* This will find the first element, and
			   position us there. */
  bson_cursor_next (c); /* This positions after the first element. */
  ok (bson_cursor_next (c) == FALSE,
      "bson_cursor_next() should bail out when encountering an invalid element.");
  bson_cursor_free (c);

  bson_free (b);
}

RUN_TEST (3, func_weird_types);
