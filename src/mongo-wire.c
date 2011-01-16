/* mongo-wire.c - libmongo-client's MongoDB wire protocoll implementation.
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

#include <glib.h>
#include <string.h>
#include <stdarg.h>

#include "bson.h"
#include "mongo-wire.h"

/** @file src/mongo-wire.c
 * Implementation of the MongoDB Wire Protocol.
 */

/** @internal Constant zero value. */
static const gint32 zero = 0;

/** @internal A MongoDB command, as it appears on the wire.
 *
 * For the sake of clarity, and sanity of the library, the header and
 * data parts are stored separately, and as such, will need to be sent
 * separately aswell.
 */
struct _mongo_packet
{
  mongo_packet_header header; /**< The packet header. */
  guint8 *data; /**< The actual data of the packet. */
  gint32 data_size; /**< Size of the data payload. */
};

/** @internal Mongo command opcodes. */
typedef enum
  {
    OP_REPLY = 1, /**< Message is a reply. Only sent by the server. */
    OP_MSG = 1000, /**< Message is a generic message. */
    OP_UPDATE = 2001, /**< Message is an update command. */
    OP_INSERT = 2002, /**< Message is an insert command. */
    OP_RESERVED = 2003, /**< Reserved and unused. */
    OP_QUERY = 2004, /**< Message is a query command. */
    OP_GET_MORE = 2005, /**< Message is a get more command. */
    OP_DELETE = 2006, /**< Message is a delete command. */
    OP_KILL_CURSORS = 2007 /**< Message is a kill cursors command. */
  } mongo_wire_opcode;

mongo_packet *
mongo_wire_packet_new (void)
{
  return (mongo_packet *)g_try_new0 (mongo_packet, 1);
}

gboolean
mongo_wire_packet_get_header (const mongo_packet *p,
			      mongo_packet_header *header)
{
  if (!p || !header)
    return FALSE;

  header->length = GINT32_FROM_LE (p->header.length);
  header->id = GINT32_FROM_LE (p->header.id);
  header->resp_to = GINT32_FROM_LE (p->header.resp_to);
  header->opcode = GINT32_FROM_LE (p->header.opcode);

  return TRUE;
}

gboolean
mongo_wire_packet_set_header (mongo_packet *p,
			      const mongo_packet_header *header)
{
  if (!p || !header)
    return FALSE;

  p->header.length = GINT32_TO_LE (header->length);
  p->header.id = GINT32_TO_LE (header->id);
  p->header.resp_to = GINT32_TO_LE (header->resp_to);
  p->header.opcode = GINT32_TO_LE (header->opcode);

  p->data_size = header->length - sizeof (mongo_packet_header);

  return TRUE;
}

gint32
mongo_wire_packet_get_data (const mongo_packet *p, const guint8 **data)
{
  if (!p || !data)
    return -1;

  *data = (const guint8 *)p->data;
  return p->data_size;
}

gboolean
mongo_wire_packet_set_data (mongo_packet *p, const guint8 *data, gint32 size)
{
  if (!p || !data || size <= 0)
    return FALSE;

  if (p->data)
    g_free (p->data);
  p->data = g_try_malloc (size);
  if (!p->data)
    return FALSE;
  memcpy (p->data, data, size);

  p->data_size = size;
  p->header.length =
    GINT32_TO_LE (p->data_size + sizeof (mongo_packet_header));

  return TRUE;
}

void
mongo_wire_packet_free (mongo_packet *p)
{
  if (!p)
    return;

  if (p->data)
    g_free (p->data);
  g_free (p);
}

mongo_packet *
mongo_wire_cmd_update (gint32 id, const gchar *ns, gint32 flags,
		       const bson *selector, const bson *update)
{
  mongo_packet *p;
  gint32 t_flags;
  gint nslen;

  if (!ns || !selector || !update)
    return NULL;

  if (bson_size (selector) < 0 ||
      bson_size (update) < 0)
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  if (!p)
    return NULL;
  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_UPDATE);

  nslen = strlen (ns) + 1;
  p->data_size = bson_size (selector) + bson_size (update) +
    sizeof (gint32) * 2 + nslen;

  p->data = g_try_malloc (p->data_size);
  if (!p->data)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  memcpy (p->data, (void *)&zero, sizeof (gint32));
  memcpy (p->data + sizeof (gint32), (void *)ns, nslen);
  memcpy (p->data + sizeof (gint32) + nslen, (void *)&t_flags,
	  sizeof (gint32));
  memcpy (p->data + sizeof (gint32) * 2 + nslen,
	  bson_data (selector), bson_size (selector));
  memcpy (p->data + sizeof (gint32) * 2 + nslen + bson_size (selector),
	  bson_data (update), bson_size (update));

  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_packet *
mongo_wire_cmd_insert (gint32 id, const gchar *ns, const bson *docs, ...)
{
  mongo_packet *p;
  va_list ap;
  bson *d;
  gint32 pos;

  if (!ns || !docs)
    return NULL;

  if (bson_size (docs) < 0)
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  if (!p)
    return NULL;
  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_INSERT);

  p->data_size = pos = sizeof (gint32) + strlen (ns) + 1 + bson_size (docs);
  p->data = (guint8 *)g_try_malloc (p->data_size);
  if (!p->data)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  memcpy (p->data, (void *)&zero, sizeof (gint32));
  memcpy (p->data + sizeof (gint32), (void *)ns, strlen (ns) + 1);
  memcpy (p->data + sizeof (gint32) + strlen (ns) + 1,
	  bson_data (docs), bson_size (docs));

  va_start (ap, docs);
  while ((d = (bson *)va_arg (ap, gpointer)))
    {
      if (bson_size (d) < 0)
	{
	  mongo_wire_packet_free (p);
	  return NULL;
	}

      p->data_size += bson_size (d);
      p->data = (guint8 *)g_try_realloc (p->data, p->data_size);
      if (!p->data)
	{
	  mongo_wire_packet_free (p);
	  return NULL;
	}
      memcpy (p->data + pos, bson_data (d), bson_size (d));
      pos += bson_size (d);
    }
  va_end (ap);

  if (!p->data)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  p->data_size = pos;
  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_packet *
mongo_wire_cmd_query (gint32 id, const gchar *ns, gint32 flags,
		      gint32 skip, gint32 ret, const bson *query,
		      const bson *sel)
{
  mongo_packet *p;
  gint32 tmp, nslen;

  if (!ns || !query)
    return NULL;

  if (bson_size (query) < 0 || (sel && bson_size (sel) < 0))
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  if (!p)
    return NULL;
  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_QUERY);

  nslen = strlen (ns) + 1;
  p->data_size =
    sizeof (gint32) + nslen + sizeof (gint32) * 2 + bson_size (query);

  if (sel)
    p->data_size += bson_size (sel);

  p->data = g_try_malloc (p->data_size);
  if (!p->data)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  tmp = GINT32_TO_LE (flags);
  memcpy (p->data, (void *)&tmp, sizeof (gint32));
  memcpy (p->data + sizeof (gint32), (void *)ns, nslen);
  tmp = GINT32_TO_LE (skip);
  memcpy (p->data + sizeof (gint32) + nslen, (void *)&tmp, sizeof (gint32));
  tmp = GINT32_TO_LE (ret);
  memcpy (p->data + sizeof (gint32) * 2 + nslen,
	  (void *)&tmp, sizeof (gint32));
  memcpy (p->data + sizeof (gint32) * 3 + nslen, bson_data (query),
	  bson_size (query));

  if (sel)
    memcpy (p->data + sizeof (gint32) * 3 + nslen + bson_size (query),
	    bson_data (sel), bson_size (sel));

  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_packet *
mongo_wire_cmd_get_more (gint32 id, const gchar *ns,
			 gint32 ret, gint64 cursor_id)
{
  mongo_packet *p;
  gint32 t_ret;
  gint64 t_cid;
  gint32 nslen;

  if (!ns)
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  if (!p)
    return NULL;

  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_GET_MORE);

  t_ret = GINT32_TO_LE (ret);
  t_cid = GINT64_TO_LE (cursor_id);

  nslen = strlen (ns) + 1;
  p->data_size = sizeof (gint32) + nslen + sizeof (gint32) + sizeof (gint64);

  p->data = g_try_malloc (p->data_size);
  if (!p->data)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  memcpy (p->data, (void *)&zero, sizeof (gint32));
  memcpy (p->data + sizeof (gint32), (void *)ns, nslen);
  memcpy (p->data + sizeof (gint32) + nslen, (void *)&t_ret, sizeof (gint32));
  memcpy (p->data + sizeof (gint32) * 2 + nslen,
	  (void *)&t_cid, sizeof (gint64));

  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_packet *
mongo_wire_cmd_delete (gint32 id, const gchar *ns,
		       gint32 flags, const bson *sel)
{
  mongo_packet *p;
  gint32 t_flags, nslen;

  if (!ns || !sel)
    return NULL;

  if (bson_size (sel) < 0)
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  if (!p)
    return NULL;

  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_DELETE);

  nslen = strlen (ns) + 1;
  p->data_size = sizeof (gint32) + nslen + sizeof (gint32) + bson_size (sel);
  t_flags = GINT32_TO_LE (flags);

  p->data = g_try_malloc (p->data_size);
  if (!p->data)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  memcpy (p->data, (void *)&zero, sizeof (gint32));
  memcpy (p->data + sizeof (gint32), (void *)ns, nslen);
  memcpy (p->data + sizeof (gint32) + nslen,
	  (void *)&t_flags, sizeof (gint32));
  memcpy (p->data + sizeof (gint32) * 2 + nslen,
	  bson_data (sel), bson_size (sel));

  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_packet *
mongo_wire_cmd_kill_cursors (gint32 id, gint32 n,
			     gint64 cursor_ids, ...)
{
  mongo_packet *p;
  gint32 i, t_n, pos;
  gint64 t_cid;
  va_list ap;

  if (n <= 0)
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  if (!p)
    return NULL;

  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_KILL_CURSORS);

  t_n = GINT32_TO_LE (n);
  t_cid = GINT64_TO_LE (cursor_ids);
  p->data_size = sizeof (gint32) + sizeof (gint32) + sizeof (gint64) * n;
  pos = sizeof (gint32) * 2 + sizeof (gint64);

  p->data = g_try_malloc (p->data_size);
  if (!p->data)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  memcpy (p->data, (void *)&zero, sizeof (gint32));
  memcpy (p->data + sizeof (gint32), (void *)&t_n, sizeof (gint32));
  memcpy (p->data + sizeof (gint32) * 2, (void *)&t_cid, sizeof (gint64));

  va_start (ap, cursor_ids);
  for (i = 1; i < n; i++)
    {
      t_cid = va_arg (ap, gint64);
      t_cid = GINT64_TO_LE (t_cid);

      memcpy (p->data + pos, (void *)&t_cid, sizeof (gint64));
      pos += sizeof (gint64);
    }
  va_end (ap);

  if (!p->data)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data_size);

  return p;
}

mongo_packet *
mongo_wire_cmd_custom (gint32 id, const gchar *db, const bson *command)
{
  mongo_packet *p;
  gchar *ns;
  bson *empty;

  if (!db || !command)
    return NULL;

  if (bson_size (command) < 0)
    return NULL;

  ns = g_strconcat (db, ".$cmd", NULL);
  if (!ns)
    return NULL;

  empty = bson_new ();
  if (!empty)
    {
      g_free (ns);
      return NULL;
    }
  bson_finish (empty);

  p = mongo_wire_cmd_query (id, ns, 0, 0, 1, command, empty);
  g_free (ns);
  return p;
}

gboolean
mongo_wire_reply_packet_get_header (const mongo_packet *p,
				    mongo_reply_packet_header *hdr)
{
  mongo_reply_packet_header h;
  const guint8 *data;

  if (!p || !hdr)
    return FALSE;

  if (p->header.opcode != OP_REPLY)
    return FALSE;

  if (mongo_wire_packet_get_data (p, &data) == -1)
    return FALSE;

  memcpy (&h, data, sizeof (mongo_reply_packet_header));

  hdr->flags = GINT32_FROM_LE (h.flags);
  hdr->cursor_id = GINT64_FROM_LE (h.cursor_id);
  hdr->start = GINT32_FROM_LE (h.start);
  hdr->returned = GINT32_FROM_LE (h.returned);

  return TRUE;
}

gboolean
mongo_wire_reply_packet_get_data (const mongo_packet *p,
				  const guint8 **data)
{
  const guint8 *d;

  if (!p || !data)
    return FALSE;

  if (p->header.opcode != OP_REPLY)
    return FALSE;

  if (mongo_wire_packet_get_data (p, &d) == -1)
    return FALSE;

  *data = d + sizeof (mongo_reply_packet_header);
  return TRUE;
}

gboolean
mongo_wire_reply_packet_get_nth_document (const mongo_packet *p,
					  gint32 n,
					  bson **doc)
{
  const guint8 *d;
  bson *b;
  mongo_reply_packet_header h;
  gint32 i;
  gint32 pos = 0;

  if (!p || !doc || n <= 0)
    return FALSE;

  if (p->header.opcode != OP_REPLY)
    return FALSE;

  if (!mongo_wire_reply_packet_get_header (p, &h))
    return FALSE;

  if (h.returned < n)
    return FALSE;

  if (!mongo_wire_reply_packet_get_data (p, &d))
    return FALSE;

  for (i = 1; i < n; i++)
    pos += GINT32_FROM_LE ((guint32)d[0]);

  b = bson_new_from_data (d + pos, GINT32_FROM_LE ((guint32)d[pos]) - 1);
  if (!b)
    return FALSE;

  *doc = b;
  return TRUE;
}
