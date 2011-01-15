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
#pragma pack(1)
  struct
  {
    gint32 length; /**< Full length of the packet, including the
		       header. */
    gint32 id; /**< Sequence ID, used when MongoDB responds to a
		  command. */
    gint32 resp_to; /**< ID the response is an answer to. Only sent
		       by the MongoDB server, never set on
		       client-side. */
    gint32 opcode; /**< The opcode of the command. @see
		      mongo_wire_opcode. <*/
  } header;

  GByteArray *data; /**< The actual data of the packet. */
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

mongo_packet *mongo_wire_cmd_query (gint32 id, const gchar *ns, gint32 flags,
				    gint32 skip, gint32 ret, const bson *query,
				    const bson *sel)
{
  mongo_packet *p;
  gint32 size, tmp;

  if (!ns || !query)
    return NULL;

  if (bson_size (query) < 0 || (sel && bson_size (sel) < 0))
    return NULL;

  p = (mongo_packet *)g_try_new0 (mongo_packet, 1);
  p->header.id = id;
  p->header.opcode = GINT32_TO_LE (OP_QUERY);

  size = sizeof (gint32) + strlen (ns) + 1 + sizeof (gint32) * 2 +
    bson_size (query);

  if (sel)
    size += bson_size (sel);

  p->data = g_byte_array_sized_new (size);
  p->data = g_byte_array_append (p->data, (guint8 *)&flags, sizeof (flags));
  p->data = g_byte_array_append (p->data, (guint8 *)ns, strlen (ns) + 1);
  tmp = GINT32_TO_LE (skip);
  p->data = g_byte_array_append (p->data, (guint8 *)&tmp, sizeof (tmp));
  tmp = GINT32_TO_LE (ret);
  p->data = g_byte_array_append (p->data, (guint8 *)&tmp, sizeof (tmp));
  p->data = g_byte_array_append (p->data, bson_data (query),
				 bson_size (query));
  if (sel)
    p->data = g_byte_array_append (p->data, bson_data (sel), bson_size (sel));
  if (!p->data)
    return NULL;

  p->header.length = GINT32_TO_LE (sizeof (p->header) + p->data->len);

  return p;
}
