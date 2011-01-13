#ifndef BALABIT_BSON_H
#define BALABIT_BSON_H 1

#include <glib.h>

typedef struct _bson bson;
typedef struct _bson_cursor bson_cursor;

typedef enum
  {
    BSON_TYPE_NONE = 0, /* Only used for errors */
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
  } bson_type;

bson *bson_new (void);
bson *bson_new_sized (gint32 size);
gboolean bson_finish (bson *b);
void bson_free (bson *b);

gint32 bson_size (const bson *b);
const guint8 *bson_data (const bson *b);

gboolean bson_append_string (bson *b, const gchar *name, const gchar *val,
			     gint32 length);
gboolean bson_append_double (bson *b, const gchar *name, gdouble d);
gboolean bson_append_document (bson *b, const gchar *name, const bson *doc);
gboolean bson_append_array (bson *b, const gchar *name, const bson *array);
gboolean bson_append_oid (bson *b, const gchar *name, const guint8 *oid);
gboolean bson_append_boolean (bson *b, const gchar *name, gboolean value);
gboolean bson_append_utc_datetime (bson *b, const gchar *name, gint64 ts);
gboolean bson_append_null (bson *b, const gchar *name);
gboolean bson_append_regex (bson *b, const gchar *name, const gchar *regexp,
			    const gchar *options);
gboolean bson_append_javascript (bson *b, const gchar *name, const gchar *js,
				 gint32 len);
gboolean bson_append_symbol (bson *b, const gchar *name, const gchar *symbol,
			     gint32 len);
gboolean bson_append_int32 (bson *b, const gchar *name, gint32 i);
gboolean bson_append_timestamp (bson *b, const gchar *name, gint64 ts);
gboolean bson_append_int64 (bson *b, const gchar *name, gint64 i);

bson_cursor *bson_find (const bson *b, const gchar *name);

bson_type bson_cursor_type (const bson_cursor *c);
const gchar *bson_cursor_key (const bson_cursor *c);

gboolean bson_cursor_get_string (const bson_cursor *c, const gchar **dest);
gboolean bson_cursor_get_double (const bson_cursor *c, gdouble *dest);
gboolean bson_cursor_get_document (const bson_cursor *c, bson **dest);
gboolean bson_cursor_get_array (const bson_cursor *c, bson **dest);
gboolean bson_cursor_get_oid (const bson_cursor *c, const guint8 **dest);
gboolean bson_cursor_get_boolean (const bson_cursor *c, gboolean *dest);
gboolean bson_cursor_get_utc_datetime (const bson_cursor *c, gint64 *dest);
gboolean bson_cursor_get_regex (const bson_cursor *c, const gchar **regex,
				const gchar **options);
gboolean bson_cursor_get_javascript (const bson_cursor *c, const gchar **dest);
gboolean bson_cursor_get_symbol (const bson_cursor *c, const gchar **dest);
gboolean bson_cursor_get_int32 (const bson_cursor *c, gint32 *dest);
gboolean bson_cursor_get_timestamp (const bson_cursor *c, gint64 *dest);
gboolean bson_cursor_get_int64 (const bson_cursor *c, gint64 *dest);

#endif
