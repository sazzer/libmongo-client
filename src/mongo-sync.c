/* mongo-sync.c - libmongo-client synchronous wrapper API
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

#include "mongo.h"

gboolean
mongo_sync_cmd_update (mongo_connection *conn,
		       const gchar *ns,
		       gint32 flags, const bson *selector,
		       const bson *update)
{
  mongo_packet *p;
  gint32 rid;

  if (!conn)
    return FALSE;

  rid = mongo_connection_get_requestid (conn) + 1;

  p = mongo_wire_cmd_update (rid, ns, flags, selector, update);
  if (!p)
    return FALSE;

  if (!mongo_packet_send (conn, p))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

gboolean
mongo_sync_cmd_insert (mongo_connection *conn,
		       const char *ns,
		       const bson *doc)
{
  mongo_packet *p;
  gint32 rid;

  if (!conn)
    return FALSE;

  rid = mongo_connection_get_requestid (conn) + 1;

  p = mongo_wire_cmd_insert (rid, ns, doc, NULL);
  if (!p)
    return FALSE;

  if (!mongo_packet_send (conn, p))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

mongo_packet *
mongo_sync_cmd_query (mongo_connection *conn,
		      const gchar *ns, gint32 flags,
		      gint32 skip, gint32 ret,
		      const bson *query, const bson *sel)
{
  mongo_packet *p;
  gint32 rid;

  mongo_packet_header h;
  mongo_reply_packet_header rh;

  if (!conn)
    return NULL;

  rid = mongo_connection_get_requestid (conn) + 1;

  p = mongo_wire_cmd_query (rid, ns, flags, skip, ret, query, sel);
  if (!p)
    return NULL;

  if (!mongo_packet_send (conn, p))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv (conn);
  if (!p)
    return NULL;

  if (!mongo_wire_packet_get_header (p, &h))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (h.resp_to != rid)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (!mongo_wire_reply_packet_get_header (p, &rh))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (rh.flags & MONGO_REPLY_FLAG_QUERY_FAIL)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  return p;
}

mongo_packet *
mongo_sync_cmd_get_more (mongo_connection *conn,
			 const gchar *ns,
			 gint32 ret, gint64 cursor_id)
{
  mongo_packet *p;
  gint32 rid;

  mongo_packet_header h;
  mongo_reply_packet_header rh;

  if (!conn)
    return NULL;

  rid = mongo_connection_get_requestid (conn) + 1;

  p = mongo_wire_cmd_get_more (rid, ns, ret, cursor_id);
  if (!p)
    return NULL;

  if (!mongo_packet_send (conn, p))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv (conn);
  if (!p)
    return NULL;

  if (!mongo_wire_packet_get_header (p, &h))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (h.resp_to != rid)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (!mongo_wire_reply_packet_get_header (p, &rh))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (rh.flags & MONGO_REPLY_FLAG_NO_CURSOR)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  return p;
}

gboolean
mongo_sync_cmd_delete (mongo_connection *conn, const gchar *ns,
		       gint32 flags, const bson *sel)
{
  mongo_packet *p;
  gint32 rid;

  if (!conn)
    return FALSE;

  rid = mongo_connection_get_requestid (conn) + 1;

  p = mongo_wire_cmd_delete (rid, ns, flags, sel);
  if (!p)
    return FALSE;

  if (!mongo_packet_send (conn, p))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

gboolean
mongo_sync_cmd_kill_cursor (mongo_connection *conn,
			    gint64 cursor_id)
{
  mongo_packet *p;
  gint32 rid;

  if (!conn)
    return FALSE;

  rid = mongo_connection_get_requestid (conn) + 1;

  p = mongo_wire_cmd_kill_cursors (rid, 1, cursor_id);
  if (!p)
    return FALSE;

  if (!mongo_packet_send (conn, p))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

mongo_packet *
mongo_sync_cmd_custom (mongo_connection *conn,
		       const gchar *db,
		       const bson *command)
{
  mongo_packet *p;
  gint32 rid;

  mongo_packet_header h;
  mongo_reply_packet_header rh;

  if (!conn)
    return NULL;

  rid = mongo_connection_get_requestid (conn) + 1;

  p = mongo_wire_cmd_custom (rid, db, command);
  if (!p)
    return NULL;

  if (!mongo_packet_send (conn, p))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv (conn);
  if (!p)
    return NULL;

  if (!mongo_wire_packet_get_header (p, &h))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (h.resp_to != rid)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (!mongo_wire_reply_packet_get_header (p, &rh))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (rh.flags & MONGO_REPLY_FLAG_QUERY_FAIL)
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  return p;
}

gdouble
mongo_sync_cmd_count (mongo_connection *conn,
		      const gchar *db, const gchar *coll,
		      const bson *query)
{
  mongo_packet *p;
  bson *cmd;
  bson_cursor *c;
  gdouble d;

  if (!conn)
    return -1;

  cmd = bson_new_sized (bson_size (query) + 32);
  bson_append_string (cmd, "count", coll, -1);
  if (query)
    bson_append_document (cmd, "query", query);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, db, cmd);
  bson_free (cmd);
  if (!p)
    return -1;

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      mongo_wire_packet_free (p);
      return -1;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  c = bson_find (cmd, "ok");
  if (!c)
    {
      bson_free (cmd);
      return -1;
    }
  if (!bson_cursor_get_double (c, &d))
    {
      bson_free (cmd);
      g_free (c);
      return -1;
    }
  g_free (c);
  if (d != 1)
    {
      bson_free (cmd);
      return -1;
    }

  c = bson_find (cmd, "n");
  if (!c)
    {
      bson_free (cmd);
      return -1;
    }
  if (!bson_cursor_get_double (c, &d))
    {
      bson_free (cmd);
      g_free (c);
      return -1;
    }
  g_free (c);
  bson_free (cmd);

  return d;
}
