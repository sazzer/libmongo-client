#include "test.h"
#include "bson.h"
#include "mongo-utils.h"

#include <glib.h>
#include <string.h>

func_config_t config;

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
test_mongo_wire_generate_reply (gboolean valid, gint32 nreturn,
				gboolean with_docs)
{
  mongo_reply_packet_header rh;
  mongo_packet_header h;
  mongo_packet *p;
  guint8 *data;
  gint data_size = sizeof (mongo_reply_packet_header);
  bson *b1 = NULL, *b2 = NULL;

  p = mongo_wire_packet_new ();

  h.opcode = (valid) ? GINT32_TO_LE (1) : GINT32_TO_LE (42);
  h.id = GINT32_TO_LE (1984);
  h.resp_to = GINT32_TO_LE (42);
  if (with_docs)
    {
      b1 = test_bson_generate_full ();
      b2 = test_bson_generate_full ();
      data_size += bson_size (b1) + bson_size (b2);
    }
  h.length = GINT32_TO_LE (sizeof (mongo_packet_header) + data_size);

  mongo_wire_packet_set_header (p, &h);

  data = g_try_malloc (data_size);

  rh.flags = 0;
  rh.cursor_id = GINT64_TO_LE ((gint64)12345);
  rh.start = 0;
  rh.returned = GINT32_TO_LE (nreturn);

  memcpy (data, &rh, sizeof (mongo_reply_packet_header));
  if (with_docs)
    {
      memcpy (data + sizeof (mongo_reply_packet_header),
	      bson_data (b1), bson_size (b1));
      memcpy (data + sizeof (mongo_reply_packet_header) + bson_size (b1),
	      bson_data (b2), bson_size (b2));
    }

  mongo_wire_packet_set_data (p, data, data_size);
  g_free (data);
  bson_free (b1);
  bson_free (b2);

  return p;
}

gboolean
test_env_setup (void)
{
  config.primary_host = config.secondary_host = NULL;
  config.primary_port = config.secondary_port = 27017;

  if (!mongo_util_parse_addr (getenv ("TEST_PRIMARY"), &config.primary_host,
			      &config.primary_port))
    return FALSE;
  mongo_util_parse_addr (getenv ("TEST_SECONDARY"), &config.secondary_host,
			 &config.secondary_port);
  return TRUE;
}

void
test_env_free (void)
{
  g_free (config.primary_host);
  g_free (config.secondary_host);
}
