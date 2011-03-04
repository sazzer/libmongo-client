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
  bson_append_timestamp (b, "ts", 1294860709000);
  bson_append_null (b, "null");
  bson_append_regex (b, "foobar", "s/foo.*bar/", "i");
  bson_append_javascript (b, "alert", "alert (\"hello world!\");", -1);
  bson_append_symbol (b, "sex", "Marilyn Monroe", -1);
  bson_append_int32 (b, "int32", 32);
  bson_append_int64 (b, "int64", (gint64)-42);
  bson_finish (b);

  bson_free (d);
  bson_free (a);

  return b;
}

mongo_packet *
test_mongo_wire_generate_reply (gboolean valid, gboolean with_doc)
{
  mongo_reply_packet_header rh;
  mongo_packet_header h;
  mongo_packet *p;
  guint8 *data;
  gint data_size = sizeof (mongo_reply_packet_header);
  bson *b = NULL;

  p = mongo_wire_packet_new ();

  h.opcode = (valid) ? 1 : 42;
  h.id = 1984;
  h.resp_to = 42;
  if (with_doc)
    {
      b = test_bson_generate_full ();
      data_size += bson_size (b);
    }
  h.length = sizeof (mongo_packet_header) + data_size;

  mongo_wire_packet_set_header (p, &h);

  data = g_try_malloc (data_size);

  rh.flags = GINT32_TO_LE (0);
  rh.cursor_id = GINT64_TO_LE (12345);
  rh.start = 0;
  rh.returned = (with_doc) ? GINT32_TO_LE (1) : 0;

  memcpy (data, &rh, sizeof (mongo_reply_packet_header));
  if (with_doc)
    memcpy (data + sizeof (mongo_reply_packet_header),
	    bson_data (b), bson_size (b));

  mongo_wire_packet_set_data (p, data, data_size);
  g_free (data);
  bson_free (b);

  return p;
}
