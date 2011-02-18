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
#include "libmongo-private.h"

/** @internal Synchronous connection object. */
struct _mongo_sync_connection
{
  mongo_connection super; /**< The parent object. */
  gboolean slaveok; /**< Whether queries against slave nodes are
		       acceptable. */
};

mongo_sync_connection *
mongo_sync_connect (const gchar *host, int port,
		    gboolean slaveok)
{
  mongo_sync_connection *s;

  s = g_try_new0 (mongo_sync_connection, 1);
  if (!s)
    return NULL;
  if (!mongo_connection_new (host, port, (mongo_connection **)&s))
    {
      mongo_sync_disconnect (s);
      return NULL;
    }
  s->slaveok = slaveok;
  return s;
}

void
mongo_sync_disconnect (mongo_sync_connection *conn)
{
  if (!conn)
    return;

  mongo_disconnect ((mongo_connection *)conn);
}

gboolean
mongo_sync_conn_get_slaveok (const mongo_sync_connection *conn)
{
  if (!conn)
    return FALSE;

  return conn->slaveok;
}

void
mongo_sync_conn_set_slaveok (mongo_sync_connection *conn,
			     gboolean slaveok)
{
  if (!conn)
    return;
  conn->slaveok = slaveok;
}

#define _SLAVE_FLAG(c) ((c->slaveok) ? MONGO_WIRE_FLAG_QUERY_SLAVE_OK : 0)

gboolean
mongo_sync_cmd_update (mongo_sync_connection *conn,
		       const gchar *ns,
		       gint32 flags, const bson *selector,
		       const bson *update)
{
  mongo_packet *p;
  gint32 rid;

  if (!conn)
    return FALSE;

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_update (rid, ns, flags, selector, update);
  if (!p)
    return FALSE;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

gboolean
mongo_sync_cmd_insert (mongo_sync_connection *conn,
		       const char *ns,
		       const bson *doc)
{
  mongo_packet *p;
  gint32 rid;

  if (!conn)
    return FALSE;

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_insert (rid, ns, doc, NULL);
  if (!p)
    return FALSE;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

mongo_packet *
mongo_sync_cmd_query (mongo_sync_connection *conn,
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

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_query (rid, ns, flags | _SLAVE_FLAG (conn),
			    skip, ret, query, sel);
  if (!p)
    return NULL;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv ((mongo_connection *)conn);
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
mongo_sync_cmd_get_more (mongo_sync_connection *conn,
			 const gchar *ns,
			 gint32 ret, gint64 cursor_id)
{
  mongo_packet *p;
  gint32 rid;

  mongo_packet_header h;
  mongo_reply_packet_header rh;

  if (!conn)
    return NULL;

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_get_more (rid, ns, ret, cursor_id);
  if (!p)
    return NULL;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv ((mongo_connection *)conn);
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
mongo_sync_cmd_delete (mongo_sync_connection *conn, const gchar *ns,
		       gint32 flags, const bson *sel)
{
  mongo_packet *p;
  gint32 rid;

  if (!conn)
    return FALSE;

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_delete (rid, ns, flags, sel);
  if (!p)
    return FALSE;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

gboolean
mongo_sync_cmd_kill_cursor (mongo_sync_connection *conn,
			    gint64 cursor_id)
{
  mongo_packet *p;
  gint32 rid;

  if (!conn)
    return FALSE;

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_kill_cursors (rid, 1, cursor_id);
  if (!p)
    return FALSE;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

mongo_packet *
mongo_sync_cmd_custom (mongo_sync_connection *conn,
		       const gchar *db,
		       const bson *command)
{
  mongo_packet *p;
  gint32 rid;

  mongo_packet_header h;
  mongo_reply_packet_header rh;

  if (!conn)
    return NULL;

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_custom (rid, db, _SLAVE_FLAG (conn), command);
  if (!p)
    return NULL;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv ((mongo_connection *)conn);
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

gboolean
_mongo_sync_check_ok (bson *b)
{
  bson_cursor *c;
  gdouble d;

  c = bson_find (b, "ok");
  if (!c)
    return FALSE;
  if (!bson_cursor_get_double (c, &d))
    {
      bson_cursor_free (c);
      return FALSE;
    }
  bson_cursor_free (c);
  return (d == 1);
}

gdouble
mongo_sync_cmd_count (mongo_sync_connection *conn,
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

  if (!_mongo_sync_check_ok (cmd))
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
      bson_cursor_free (c);
      return -1;
    }
  bson_cursor_free (c);
  bson_free (cmd);

  return d;
}

gboolean
mongo_sync_cmd_drop (mongo_sync_connection *conn,
		     const gchar *db, const gchar *coll)
{
  mongo_packet *p;
  bson *cmd;

  if (!conn)
    return FALSE;

  cmd = bson_new_sized (64);
  bson_append_string (cmd, "drop", coll, -1);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, db, cmd);
  bson_free (cmd);
  if (!p)
    return FALSE;

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  if (!_mongo_sync_check_ok (cmd))
    {
      bson_free (cmd);
      return FALSE;
    }
  bson_free (cmd);
  return TRUE;
}

gboolean
mongo_sync_cmd_get_last_error (mongo_sync_connection *conn,
			       const gchar *db, gchar **error)
{
  mongo_packet *p;
  bson *cmd;
  bson_cursor *c;
  const gchar *err;

  if (!conn)
    return FALSE;

  cmd = bson_new_sized (64);
  bson_append_int32 (cmd, "getlasterror", 1);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, db, cmd);
  bson_free (cmd);
  if (!p)
    return FALSE;

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  if (!_mongo_sync_check_ok (cmd))
    {
      bson_free (cmd);
      return FALSE;
    }

  c = bson_find (cmd, "err");
  if (!c)
    {
      bson_free (cmd);
      return FALSE;
    }
  if (bson_cursor_type (c) == BSON_TYPE_NULL)
    {
      *error = NULL;
      bson_free (cmd);
      bson_cursor_free (c);
      return TRUE;
    }
  else if (bson_cursor_type (c) == BSON_TYPE_STRING)
    {
      bson_cursor_get_string (c, &err);
      *error = g_strdup (err);
      bson_free (cmd);
      bson_cursor_free (c);
      return TRUE;
    }
  bson_free (cmd);
  bson_cursor_free (c);
  return FALSE;
}

gboolean
mongo_sync_cmd_reset_error (mongo_sync_connection *conn,
			    const gchar *db)
{
  mongo_packet *p;
  bson *cmd;

  if (!conn)
    return FALSE;

  cmd = bson_new_sized (32);
  bson_append_int32 (cmd, "reseterror", 1);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, db, cmd);
  bson_free (cmd);
  if (!p)
    return FALSE;

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      mongo_wire_packet_free (p);
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  if (!_mongo_sync_check_ok (cmd))
    {
      bson_free (cmd);
      return FALSE;
    }
  bson_free (cmd);
  return TRUE;
}
