#include <glib.h>
#include <string.h>

#include "bson.h"
#include "mongo_wire.h"

static const gint32 zero = 0;

struct _mongo_packet
{
#pragma pack(1)
  struct
  {
    gint32 length;
    gint32 id;
    gint32 resp_to;
    gint32 opcode;
  } header;

  GByteArray *data;
};

typedef enum
  {
    OP_REPLY = 1,
    OP_MSG = 1000,
    OP_UPDATE = 2001,
    OP_INSERT = 2002,
    OP_RESERVED = 2003,
    OP_QUERY = 2004,
    OP_GET_MORE = 2005,
    OP_DELETE = 2006,
    OP_KILL_CURSORS = 2007
  } mongo_wire_opcode;

gint32
mongo_wire_packet_get_header (const mongo_packet *p, const guint8 **header)
{
  if (!p || !header)
    return -1;

  *header = (const guint8 *)&p->header;
  return sizeof (p->header);
}

gint32
mongo_wire_packet_get_data (const mongo_packet *p, const guint8 **data)
{
  if (!p || !data)
    return -1;

  *data = (const guint8 *)p->data->data;
  return p->data->len;
}

void
mongo_wire_packet_free (mongo_packet *p)
{
  if (!p)
    return;

  g_byte_array_free (p->data, TRUE);
  g_free (p);
}

mongo_packet *
mongo_wire_cmd_update (gint32 id, const gchar *ns, gint32 flags,
		       const bson *selector, const bson *update)
{
  mongo_packet *p;

  if (!ns || !selector || !update)
    return NULL;

  if (bson_size (selector) < 0 ||
      bson_size (update) < 0)
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_UPDATE);

  p->data = g_byte_array_sized_new
    (bson_size (selector) + bson_size (update) +
     sizeof (gint32) + strlen (ns) + 1 + sizeof (flags));
  p->data = g_byte_array_append (p->data, (guint8 *)&zero, sizeof (zero));
  p->data = g_byte_array_append (p->data, (guint8 *)ns, strlen (ns) + 1);
  p->data = g_byte_array_append (p->data, (guint8 *)&flags, sizeof (flags));
  p->data = g_byte_array_append (p->data, bson_data (selector),
				 bson_size (selector));
  p->data = g_byte_array_append (p->data, bson_data (update),
				 bson_size (update));
  if (!p->data)
    return NULL;

  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data->len);

  return p;
}

mongo_packet *
mongo_wire_cmd_insert (gint32 id, const gchar *ns, const bson *doc)
{
  mongo_packet *p;

  if (!ns || !doc)
    return NULL;

  if (bson_size (doc) < 0)
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_INSERT);

  p->data = g_byte_array_sized_new
    (sizeof (gint32) + strlen (ns) + 1 + bson_size (doc));
  p->data = g_byte_array_append (p->data, (guint8 *)&zero, sizeof (zero));
  p->data = g_byte_array_append (p->data, (guint8 *)ns, strlen (ns) + 1);
  p->data = g_byte_array_append (p->data, bson_data (doc), bson_size (doc));
  if (!p->data)
    return NULL;

  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data->len);

  return p;
}

