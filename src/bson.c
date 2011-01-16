/* bson.c - libmongo-client's BSON implementation
 * Copyright 2011 Gergely Nagy <algernon@balabit.hu>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file src/bson.c
 * Implementation of the #BSON API.
 */

#include <glib.h>
#include <string.h>

#include "bson.h"

/** @internal BSON structure.
 */
struct _bson
{
  GByteArray *data; /**< The actual data of the BSON object. */
  gboolean finished; /**< Flag to indicate whether the object is open
			or finished. */
};

/** @internal BSON cursor structure.
 */
struct _bson_cursor
{
  const bson *obj; /**< The BSON object this is a cursor for. */
  const gchar *key; /**< Pointer within the BSON object to the
		       current key. */
  gint32 pos; /**< Position within the BSON object, pointing at the
		 element type. */
  gint32 value_pos; /**< The start of the value within the BSON
		       object, pointing right after the end of the
		       key. */
};

/** @internal Convenience macro to check data integrity.
 *
 * This macro is used whenever we want to make sure that memory
 * allocation did not fail, and our data member is still intact.
 */
#define DATA_OK(b) (b->data) ? TRUE : FALSE

/** @internal Append a byte to a BSON stream.
 *
 * @param b is the BSON stream to append to.
 * @param byte is the byte to append.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
static inline gboolean
_bson_append_byte (bson *b, const guint8 byte)
{
  b->data = g_byte_array_append (b->data, &byte, sizeof (byte));
  return DATA_OK (b);
}

/** @internal Append a 32-bit integer to a BSON stream.
 *
 * @param b is the BSON stream to append to.
 * @param i is the integer to append.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
static inline gboolean
_bson_append_int32 (bson *b, const gint32 i)
{
  b->data = g_byte_array_append (b->data, (const guint8 *)&i, sizeof (gint32));
  return DATA_OK (b);
}

/** @internal Append a 64-bit integer to a BSON stream.
 *
 * @param b is the BSON stream to append to.
 * @param i is the integer to append.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
static inline gboolean
_bson_append_int64 (bson *b, const gint64 i)
{
  b->data = g_byte_array_append (b->data, (const guint8 *)&i, sizeof (gint64));
  return DATA_OK (b);
}

/** @internal Append an element header to a BSON stream.
 *
 * The element header is a single byte, signaling the type of the
 * element, followed by a NULL-terminated C string: the key (element)
 * name.
 *
 * @param b is the BSON object to append to.
 * @param type is the element type to append.
 * @param name is the key name.
 *
 * @returns TRUE on success, FALSE otherwise.
 *
 * @note If the append fails mid-way, the data size is not rolled
 * back, thus if this function fails, the object MUST be treated as
 * invalid, and freed at the first opportunity, never accessed or
 * written to.
 */
static inline gboolean
_bson_append_element_header (bson *b, bson_type type, const gchar *name)
{
  if (!name)
    return FALSE;

  if (b->finished)
    return FALSE;

  if (!_bson_append_byte (b, (guint8) type))
    return FALSE;

  b->data = g_byte_array_append (b->data, (const guint8 *)name,
				 strlen (name) + 1);

  return DATA_OK (b);
}

/** @internal Append a string-like element to a BSON object.
 *
 * There are a few string-like elements in the BSON spec that differ
 * only in type, not in structure. This convenience function is used
 * to append them with the appropriate type.
 *
 * @param b is the BSON object to append to.
 * @param type is the string-like type to append.
 * @param name is the key name.
 * @param val is the value to append.
 * @param length is the length of the value.
 *
 * @note Passing @a -1 as length will use the full length of @a
 * val.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean
_bson_append_string_element (bson *b, bson_type type, const gchar *name,
			     const gchar *val, gint32 length)
{
  gint32 len;

  if (!val || !length)
    return FALSE;

  len = (length != -1) ? length + 1: strlen (val) + 1;

  if (!_bson_append_element_header (b, type, name))
    return FALSE;

  if (!_bson_append_int32 (b, GINT32_TO_LE (len)))
    return FALSE;

  b->data = g_byte_array_append (b->data, (const guint8 *)val, len - 1);
  if (!b->data)
    return FALSE;

  return _bson_append_byte (b, 0);
}

/** @internal Append a document-like element to a BSON object.
 *
 * Arrays and documents are both similar, and differ very little:
 * different type, and arrays have restrictions on key names (which
 * are not enforced by this library).
 *
 * This convenience function can append both types.
 *
 * @param b is the BSON object to append to.
 * @param type is the document-like type to append.
 * @param name is the key name.
 * @param doc is the document-like object to append.
 *
 * @note The @a doc must be a finished BSON object.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
static gboolean
_bson_append_document_element (bson *b, bson_type type, const gchar *name,
			       const bson *doc)
{
  if (bson_size (doc) < 0)
    return FALSE;

  if (!_bson_append_element_header (b, type, name))
    return FALSE;

  b->data = g_byte_array_append (b->data, bson_data (doc), bson_size (doc));
  return DATA_OK (b);
}

/** @internal Append a 64-bit integer to a BSON object.
 *
 * @param b is the BSON object to append to.
 * @param type is the int64-like type to append.
 * @param name is the key name.
 * @param i is the 64-bit value to append.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
static inline gboolean
_bson_append_int64_element (bson *b, bson_type type, const gchar *name,
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

bson *
bson_new_from_data (const guint8 *data, gint32 size)
{
  bson *b;

  if (!data || size <= 0)
    return NULL;

  b = g_try_new0 (bson, 1);
  if (!b)
    return NULL;

  b->data = g_byte_array_sized_new (size + 1);
  if (!b->data)
    {
      g_free (b);
      return NULL;
    }

  b->data = g_byte_array_append (b->data, data, size);
  if (!b->data)
    {
      g_free (b);
      return NULL;
    }

  return b;
}

gboolean
bson_finish (bson *b)
{
  gint32 *i;

  if (!b)
    return FALSE;

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
  if (!b)
    return -1;

  if (b->finished)
    return b->data->len;
  else
    return -1;
}

const guint8 *
bson_data (const bson *b)
{
  if (!b)
    return NULL;

  if (b->finished)
    return b->data->data;
  else
    return NULL;
}

gboolean
bson_reset (bson *b)
{
  if (!b)
    return FALSE;

  b->finished = FALSE;
  g_byte_array_set_size (b->data, 0);
  return _bson_append_int32 (b, 0);
}

void
bson_free (bson *b)
{
  if (!b)
    return;

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
bson_append_oid (bson *b, const gchar *name, const guint8 *oid)
{
  if (!oid)
    return FALSE;

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
  if (!regexp || !options)
    return FALSE;

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

/*
 * Find & retrieve data
 */
bson_cursor *
bson_cursor_new (const bson *b)
{
  bson_cursor *c;

  if (bson_size (b) == -1)
    return NULL;

  c = (bson_cursor *)g_try_new0 (bson_cursor, 1);
  if (!c)
    return NULL;

  c->obj = b;

  return c;
}

/** @internal Figure out the block size of a given type.
 *
 * Provided a #bson_type and some raw data, figures out the length of
 * the block, counted from rigth after the element name's position.
 *
 * @param type is the type of object we need the size for.
 * @param data is the raw data (starting right after the element's
 * name).
 *
 * @returns The size of the block, or -1 on error.
 *
 * @todo Figuring out the size of a regexp block is not implemented
 * yet.
 */
static gint32
_bson_get_block_size (bson_type type, const guint8 *data)
{
  switch (type)
    {
    case BSON_TYPE_STRING:
    case BSON_TYPE_JS_CODE:
    case BSON_TYPE_SYMBOL:
    case BSON_TYPE_JS_CODE_W_SCOPE:
      return GINT32_FROM_LE ((gint32)data[0]) + sizeof (gint32);
    case BSON_TYPE_DOCUMENT:
    case BSON_TYPE_ARRAY:
      return GINT32_FROM_LE ((gint32)data[0]);
    case BSON_TYPE_DOUBLE:
      return sizeof (gdouble);
    case BSON_TYPE_BINARY:
      return GINT32_FROM_LE ((gint32)data[0]) + 1;
    case BSON_TYPE_OID:
      return 12;
    case BSON_TYPE_BOOLEAN:
      return 1;
    case BSON_TYPE_UTC_DATETIME:
    case BSON_TYPE_TIMESTAMP:
    case BSON_TYPE_INT64:
      return sizeof (gint64);
    case BSON_TYPE_NULL:
      return 0;
    case BSON_TYPE_REGEXP:
      g_warning ("BSON_TYPE_REGEXP block size calculation is not implemented yet");
      return -1;
    case BSON_TYPE_INT32:
      return sizeof (gint32);
    default:
      return -1;
    }
}

gboolean
bson_cursor_next (bson_cursor *c)
{
  const guint8 *d;
  gint32 pos;

  if (!c)
    return FALSE;

  d = bson_data (c->obj);

  if (c->pos == 0)
    pos = sizeof (guint32);
  else
    pos = c->value_pos +
      _bson_get_block_size (bson_cursor_type (c), d + c->value_pos);

  if (pos >= bson_size (c->obj) - 1)
    return FALSE;

  c->pos = pos;
  c->key = (gchar *) &d[c->pos + 1];
  c->value_pos = c->pos + strlen (c->key) + 2;

  return TRUE;
}

bson_cursor *
bson_find (const bson *b, const gchar *name)
{
  gint32 pos = sizeof (guint32);
  const guint8 *d;

  if (bson_size (b) == -1 || !name)
    return NULL;

  d = bson_data (b);

  while (pos < bson_size (b) - 1)
    {
      bson_type t = (bson_type) d[pos];
      const gchar *key = (gchar *) &d[pos + 1];
      gint32 value_pos = pos + strlen (key) + 2;

      if (!strcmp (key, name))
	{
	  bson_cursor *c;

	  c = (bson_cursor *)g_try_new0 (bson_cursor, 1);
	  if (!c)
	    return NULL;

	  c->obj = b;
	  c->key = key;
	  c->pos = pos;
	  c->value_pos = value_pos;

	  return c;
	}
      pos = value_pos + _bson_get_block_size (t, &d[value_pos]);
    }

  return NULL;
}

bson_type
bson_cursor_type (const bson_cursor *c)
{
  if (!c || c->pos < sizeof (gint32))
    return BSON_TYPE_NONE;

  return (bson_type)(bson_data (c->obj)[c->pos]);
}

const gchar *
bson_cursor_key (const bson_cursor *c)
{
  if (!c)
    return NULL;

  return c->key;
}

/** @internal Convenience macro to verify a cursor's type.
 *
 * Verifies that the cursor's type is the same as the type requested
 * by the caller, and returns FALSE if there is a mismatch.
 */
#define BSON_CURSOR_CHECK_TYPE(c,type)		\
  if (bson_cursor_type(c) != type)		\
    return FALSE;

gboolean
bson_cursor_get_string (const bson_cursor *c, const gchar **dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_STRING);

  *dest = (gchar *)(bson_data (c->obj) + c->value_pos + sizeof (gint32));

  return TRUE;
}

gboolean
bson_cursor_get_double (const bson_cursor *c, gdouble *dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_DOUBLE);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (gdouble));

  return TRUE;
}

gboolean
bson_cursor_get_document (const bson_cursor *c, bson **dest)
{
  bson *b;
  gint32 size;

  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_DOCUMENT);

  size = GINT32_FROM_LE ((gint32)(bson_data (c->obj)[c->value_pos]));
  b = bson_new_sized (size);
  b->data = g_byte_array_append (b->data,
				 bson_data (c->obj) + c->value_pos +
				 sizeof (gint32), size);
  bson_finish (b);

  *dest = b;

  return TRUE;
}

gboolean
bson_cursor_get_array (const bson_cursor *c, bson **dest)
{
  bson *b;
  gint32 size;

  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_ARRAY);

  size = GINT32_FROM_LE ((gint32)(bson_data (c->obj)[c->value_pos]));
  b = bson_new_sized (size);
  b->data = g_byte_array_append (b->data,
				 bson_data (c->obj) + c->value_pos +
				 sizeof (gint32), size);
  bson_finish (b);

  *dest = b;

  return TRUE;
}

gboolean
bson_cursor_get_oid (const bson_cursor *c, const guint8 **dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_OID);

  *dest = (guint8 *)(bson_data (c->obj) + c->value_pos);

  return TRUE;
}

gboolean
bson_cursor_get_boolean (const bson_cursor *c, gboolean *dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_BOOLEAN);

  *dest = FALSE;
  memcpy (dest, bson_data (c->obj) + c->value_pos, 1);

  return TRUE;
}

gboolean
bson_cursor_get_utc_datetime (const bson_cursor *c,
			      gint64 *dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_UTC_DATETIME);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (gint64));
  *dest = GINT64_FROM_LE (*dest);

  return TRUE;
}

/** @todo Not implemented yet! */
gboolean
bson_cursor_get_regex (const bson_cursor *c, const gchar **regex,
		       const gchar **options)
{
  if (!regex || !options)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_REGEXP);

  g_warning ("bson_cursor_get_regex is not implemented yet");
  return FALSE;
}

gboolean
bson_cursor_get_javascript (const bson_cursor *c, const gchar **dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_JS_CODE);

  *dest = (gchar *)(bson_data (c->obj) + c->value_pos + sizeof (gint32));

  return TRUE;
}

gboolean
bson_cursor_get_symbol (const bson_cursor *c, const gchar **dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_SYMBOL);

  *dest = (gchar *)(bson_data (c->obj) + c->value_pos + sizeof (gint32));

  return TRUE;
}

gboolean
bson_cursor_get_int32 (const bson_cursor *c, gint32 *dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_INT32);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (gint32));
  *dest = GINT32_FROM_LE (*dest);

  return TRUE;
}

gboolean
bson_cursor_get_timestamp (const bson_cursor *c, gint64 *dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_TIMESTAMP);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (gint64));
  *dest = GINT64_FROM_LE (*dest);

  return TRUE;
}

gboolean
bson_cursor_get_int64 (const bson_cursor *c, gint64 *dest)
{
  if (!dest)
    return FALSE;

  BSON_CURSOR_CHECK_TYPE (c, BSON_TYPE_INT64);

  memcpy (dest, bson_data (c->obj) + c->value_pos, sizeof (gint64));
  *dest = GINT64_FROM_LE (*dest);

  return TRUE;
}
