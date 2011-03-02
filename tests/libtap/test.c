#include "test.h"
#include "bson.h"

#include <glib.h>
#include <string.h>

bson *
test_bson_generate_full (void)
{
  bson *b, *d, *a;
  guint8 oid[] = "1234567890ab";

  a = bson_new ();
  bson_append_int32 (a, "0", 32);
  bson_append_int64 (a, "1", (gint64)-42);
  bson_finish (a);

  d = bson_new ();
  bson_append_string (d, "name", "sub-document", -1);
  bson_append_int32 (d, "answer", 42);
  bson_finish (d);

  b = bson_new ();
  bson_append_double (b, "double", 3.14);
  bson_append_string (b, "str", "hello world", -1);
  bson_append_document (b, "doc", d);
  bson_append_array (b, "array", a);
  bson_append_binary (b, "binary0", BSON_BINARY_SUBTYPE_GENERIC,
		      (guint8 *)"foo\0bar", 7);
  bson_append_oid (b, "_id", oid);
  bson_append_boolean (b, "TRUE", FALSE);
  bson_append_utc_datetime (b, "date", 1294860709000);
  bson_append_null (b, "null");
  bson_append_regex (b, "foobar", "s/foo.*bar/", "i");
  bson_append_javascript (b, "alert", "alert (\"hello world!\");", -1);
  bson_append_symbol (b, "sex", "Marylin Monroe", -1);
  bson_append_int32 (b, "int32", 32);
  bson_append_int64 (b, "int64", (gint64)-42);
  bson_finish (b);

  bson_free (d);
  bson_free (a);

  return b;
}
