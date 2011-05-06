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
}

RUN_TEST (1, func_weird_types);
