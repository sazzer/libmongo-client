#include <glib.h>
#include <string.h>

#include "bson.h"

struct _bson
{
  GByteArray *data;
  gboolean finished;
};

enum
  {
    BSON_TYPE_DOUBLE = 0x01,
    BSON_TYPE_STRING,
    BSON_TYPE_DOCUMENT,
    BSON_TYPE_ARRAY,
    BSON_TYPE_BINARY,
    BSON_TYPE_UNDEFINED, /* Deprecated*/
    BSON_TYPE_OID,
    BSON_TYPE_BOOLEAN,
    BSON_TYPE_UTC_DATETIME,
    BSON_TYPE_NULL,
    BSON_TYPE_REGEXP,
    BSON_TYPE_DBPOINTER, /* Deprecated */
    BSON_TYPE_JS_CODE,
    BSON_TYPE_SYMBOL,
    BSON_TYPE_JS_CODE_W_SCOPE,
    BSON_TYPE_INT32,
    BSON_TYPE_TIMESTAMP,
    BSON_TYPE_INT64,
    BSON_TYPE_MIN = 0xff,
    BSON_TYPE_MAX = 0x7f
  };

#define DATA_OK(b) (b->data) ? TRUE : FALSE

static inline gboolean
_bson_append_byte (bson *b, const guint8 byte)
{
  b->data = g_byte_array_append (b->data, &byte, sizeof (byte));
  return DATA_OK (b);
}

static inline gboolean
_bson_append_int32 (bson *b, const gint32 i)
{
  b->data = g_byte_array_append (b->data, (const guint8 *)&i, sizeof (gint32));
  return DATA_OK (b);
}

static inline gboolean
_bson_append_int64 (bson *b, const gint64 i)
{
  b->data = g_byte_array_append (b->data, (const guint8 *)&i, sizeof (gint64));
  return DATA_OK (b);
}

static inline gboolean
_bson_append_element_header (bson *b, guint8 type, const gchar *name)
{
  if (b->finished)
    return FALSE;

  if (!_bson_append_byte (b, type))
    return FALSE;

  b->data = g_byte_array_append (b->data, (const guint8 *)name,
				 strlen (name) + 1);

  return DATA_OK (b);
}

gboolean
_bson_append_string_element (bson *b, guint8 type, const gchar *name,
			     const gchar *val, gint32 length)
{
  gint32 len = (length != -1) ? length + 1: strlen (val) + 1;

  if (!_bson_append_element_header (b, type, name))
    return FALSE;

  if (!_bson_append_int32 (b, GINT32_TO_LE (len)))
    return FALSE;

  b->data = g_byte_array_append (b->data, (const guint8 *)val, len - 1);
  if (!b->data)
    return FALSE;

  return _bson_append_byte (b, 0);
}

static gboolean
_bson_append_document_element (bson *b, guint8 type, const gchar *name,
			       const bson *doc)
{
  if (bson_size (doc) < 0)
    return FALSE;

  if (!_bson_append_element_header (b, type, name))
    return FALSE;

  b->data = g_byte_array_append (b->data, bson_data (doc), bson_size (doc));
  return DATA_OK (b);
}

static inline gboolean
_bson_append_int64_element (bson *b, guint8 type, const gchar *name,
			    gint64 i)
{
  if (!_bson_append_element_header (b, type, name))
    return FALSE;

  return _bson_append_int64 (b, GINT64_TO_LE (i));
}


/********************
 * Public interface *
 ********************/

bson *
bson_new (void)
{
  return bson_new_sized (0);
}

bson *
bson_new_sized (gint32 size)
{
  bson *b = g_try_new0 (bson, 1);

  if (!b)
    return NULL;

  b->data = g_byte_array_sized_new (size + 1);
  if (!b->data)
    {
      g_free (b);
      return NULL;
    }

  if (!_bson_append_int32 (b, 0))
    {
      bson_free (b);
      return NULL;
    }

  return b;
}

gboolean
bson_finish (bson *b)
{
  gint32 *i;

  if (b->finished)
    return TRUE;

  if (!_bson_append_byte (b, 0))
    return FALSE;

  i = (gint32 *) (&b->data->data[0]);
  *i = GINT32_TO_LE ((gint32) (b->data->len));

  b->finished = TRUE;

  return TRUE;
}

gint32
bson_size (const bson *b)
{
  if (b->finished)
    return b->data->len;
  else
    return -1;
}

const guint8 *
bson_data (const bson *b)
{
  if (b->finished)
    return b->data->data;
  else
    return NULL;
}

void
bson_free (bson *b)
{
  g_byte_array_free (b->data, TRUE);
  g_free (b);
}

/*
 * Append elements
 */

gboolean
bson_append_double (bson *b, const gchar *name, gdouble val)
{
  if (!_bson_append_element_header (b, BSON_TYPE_DOUBLE, name))
    return FALSE;

  b->data = g_byte_array_append (b->data, (const guint8 *)&val, sizeof (val));
  return DATA_OK (b);
}

gboolean
bson_append_string (bson *b, const gchar *name, const gchar *val,
		    gint32 length)
{
  return _bson_append_string_element (b, BSON_TYPE_STRING, name, val, length);
}

gboolean
bson_append_document (bson *b, const gchar *name, const bson *doc)
{
  return _bson_append_document_element (b, BSON_TYPE_DOCUMENT, name, doc);
}

gboolean
bson_append_array (bson *b, const gchar *name, const bson *array)
{
  return _bson_append_document_element (b, BSON_TYPE_ARRAY, name, array);
}

gboolean
bson_append_binary (bson *b, const gchar *name, guint8 subtype,
		    const guint8 *data, gint32 size)
{
  return FALSE;
}

gboolean
bson_append_oid (bson *b, const gchar *name, const guint8 *oid)
{
  if (!_bson_append_element_header (b, BSON_TYPE_OID, name))
    return FALSE;

  b->data = g_byte_array_append (b->data, oid, 12);
  return DATA_OK (b);
}

gboolean
bson_append_boolean (bson *b, const gchar *name, gboolean value)
{
  if (!_bson_append_element_header (b, BSON_TYPE_BOOLEAN, name))
    return FALSE;

  return _bson_append_byte (b, (guint8)value);
}

gboolean
bson_append_utc_datetime (bson *b, const gchar *name, gint64 ts)
{
  return _bson_append_int64_element (b, BSON_TYPE_UTC_DATETIME, name, ts);
}

gboolean
bson_append_null (bson *b, const gchar *name)
{
  return _bson_append_element_header (b, BSON_TYPE_NULL, name);
}

gboolean
bson_append_regex (bson *b, const gchar *name, const gchar *regexp,
		   const gchar *options)
{
  if (!_bson_append_element_header (b, BSON_TYPE_REGEXP, name))
    return FALSE;

  b->data = g_byte_array_append (b->data, (const guint8 *)regexp,
				 strlen (regexp) + 1);
  b->data = g_byte_array_append (b->data, (const guint8 *)options,
				 strlen (options) + 1);

  return DATA_OK (b);
}

gboolean
bson_append_javascript (bson *b, const gchar *name, const gchar *js,
			gint32 len)
{
  return _bson_append_string_element (b, BSON_TYPE_JS_CODE, name, js, len);
}

gboolean
bson_append_symbol (bson *b, const gchar *name, const gchar *symbol,
		    gint32 len)
{
  return _bson_append_string_element (b, BSON_TYPE_SYMBOL, name, symbol, len);
}

gboolean
bson_append_javascript_w_scope (bson *b, const gchar *name, const gchar *js,
				gint32 len, bson *doc)
{
  return FALSE;
}

gboolean
bson_append_int32 (bson *b, const gchar *name, gint32 i)
{
  if (!_bson_append_element_header (b, BSON_TYPE_INT32, name))
    return FALSE;

  return _bson_append_int32 (b, GINT32_TO_LE (i));
}

gboolean
bson_append_timestamp (bson *b, const gchar *name, gint64 ts)
{
  if (!_bson_append_element_header (b, BSON_TYPE_TIMESTAMP, name))
    return FALSE;

  return _bson_append_int64 (b, GINT64_TO_LE (ts));
}

gboolean
bson_append_int64 (bson *b, const gchar *name, gint64 i)
{
  return _bson_append_int64_element (b, BSON_TYPE_INT64, name, i);
}
