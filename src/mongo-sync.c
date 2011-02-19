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

#include <stdlib.h>
#include <errno.h>
#include <string.h>

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
      int e = errno;

      mongo_sync_disconnect (s);
      errno = e;
      return NULL;
    }
  s->slaveok = slaveok;
  return s;
}

mongo_sync_connection *
mongo_sync_reconnect (mongo_sync_connection *conn,
		      gboolean force_master)
{
  gboolean ping = FALSE;
  guint i;
  mongo_sync_connection *nc;
  gchar *host;
  gint port;

  if (!conn)
    {
      errno = ENOTCONN;
      return NULL;
    }

  ping = mongo_sync_cmd_ping (conn);

  if (ping)
    {
      if (!force_master)
	return conn;
      if (force_master && mongo_sync_cmd_is_master (conn))
	return conn;

      /* Force refresh the host list. */
      mongo_sync_cmd_is_master (conn);
    }

  /* We either didn't ping, or we're not master, and have to
   * reconnect.
   *
   * First, check if we have a primary, and if we can connect there.
   */

  if (conn->rs.primary)
    {
      if (mongo_util_parse_addr (conn->rs.primary, &host, &port))
	{
	  nc = mongo_sync_connect (host, port, conn->slaveok);
	  g_free (host);
	  if (nc)
	    {
	      int e;

	      /* We can call ourselves here, since connect does not set
		 conn->rs, thus, we won't end up in an infinite loop. */
	      nc = mongo_sync_reconnect (nc, force_master);
	      e = errno;
	      mongo_sync_disconnect (conn);
	      errno = e;
	      return nc;
	    }
	}
    }

  /* No primary found, or we couldn't connect, try the rest of the
     hosts. */

  for (i = 0; i < g_list_length (conn->rs.hosts); i++)
    {
      gchar *addr = (gchar *)g_list_nth_data (conn->rs.hosts, i);
      int e;

      if (!mongo_util_parse_addr (addr, &host, &port))
	continue;

      nc = mongo_sync_connect (host, port, conn->slaveok);
      g_free (host);
      if (!nc)
	continue;

      nc = mongo_sync_reconnect (nc, force_master);
      e = errno;
      mongo_sync_disconnect (conn);
      errno = e;
      return (nc);
    }

  mongo_sync_disconnect (conn);
  errno = EHOSTUNREACH;
  return NULL;
}

void
mongo_sync_disconnect (mongo_sync_connection *conn)
{
  GList *l;

  if (!conn)
    return;

  g_free (conn->rs.primary);

  /* Delete the host list. */
  l = conn->rs.hosts;
  while (l)
    {
      g_free (l->data);
      l = g_list_delete_link (l, l);
    }

  mongo_disconnect ((mongo_connection *)conn);
}

gboolean
mongo_sync_conn_get_slaveok (const mongo_sync_connection *conn)
{
  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }

  return conn->slaveok;
}

void
mongo_sync_conn_set_slaveok (mongo_sync_connection *conn,
			     gboolean slaveok)
{
  if (!conn)
    {
      errno = ENOTCONN;
      return;
    }

  errno = 0;
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
    {
      errno = ENOTCONN;
      return FALSE;
    }

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_update (rid, ns, flags, selector, update);
  if (!p)
    return FALSE;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

gboolean
mongo_sync_cmd_insert (mongo_sync_connection *conn,
		       const char *ns, ...)
{
  mongo_packet *p;
  gint32 rid;
  va_list ap;

  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }
  if (!ns)
    {
      errno = EINVAL;
      return FALSE;
    }

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  va_start (ap, ns);
  p = mongo_wire_cmd_insert_va (rid, ns, ap);
  if (!p)
    {
      int e = errno;

      va_end (ap);
      errno = e;
      return FALSE;
    }
  va_end (ap);

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
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
    {
      errno = ENOTCONN;
      return NULL;
    }

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_query (rid, ns, flags | _SLAVE_FLAG (conn),
			    skip, ret, query, sel);
  if (!p)
    return NULL;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv ((mongo_connection *)conn);
  if (!p)
    return NULL;

  if (!mongo_wire_packet_get_header (p, &h))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  if (h.resp_to != rid)
    {
      mongo_wire_packet_free (p);
      errno = EPROTO;
      return NULL;
    }

  if (!mongo_wire_reply_packet_get_header (p, &rh))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  if (rh.flags & MONGO_REPLY_FLAG_QUERY_FAIL)
    {
      mongo_wire_packet_free (p);
      errno = ENOENT;
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
    {
      errno = ENOTCONN;
      return NULL;
    }

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_get_more (rid, ns, ret, cursor_id);
  if (!p)
    return NULL;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv ((mongo_connection *)conn);
  if (!p)
    return NULL;

  if (!mongo_wire_packet_get_header (p, &h))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  if (h.resp_to != rid)
    {
      mongo_wire_packet_free (p);
      errno = EPROTO;
      return NULL;
    }

  if (!mongo_wire_reply_packet_get_header (p, &rh))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  if (rh.flags & MONGO_REPLY_FLAG_NO_CURSOR)
    {
      mongo_wire_packet_free (p);
      errno = ESTALE;
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
    {
      errno = ENOTCONN;
      return FALSE;
    }

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_delete (rid, ns, flags, sel);
  if (!p)
    return FALSE;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  return TRUE;
}

gboolean
mongo_sync_cmd_kill_cursors (mongo_sync_connection *conn,
			     gint32 n, ...)
{
  mongo_packet *p;
  gint32 rid;
  va_list ap;

  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }
  if (n <= 0)
    {
      errno = EINVAL;
      return FALSE;
    }

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  va_start (ap, n);
  p = mongo_wire_cmd_kill_cursors_va (rid, n, ap);
  if (!p)
    {
      int e = errno;

      va_end (ap);
      errno = e;
      return FALSE;
    }
  va_end (ap);

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
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
    {
      errno = ENOTCONN;
      return NULL;
    }

  rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

  p = mongo_wire_cmd_custom (rid, db, _SLAVE_FLAG (conn), command);
  if (!p)
    return NULL;

  if (!mongo_packet_send ((mongo_connection *)conn, p))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }
  mongo_wire_packet_free (p);

  p = mongo_packet_recv ((mongo_connection *)conn);
  if (!p)
    return NULL;

  if (!mongo_wire_packet_get_header (p, &h))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  if (h.resp_to != rid)
    {
      mongo_wire_packet_free (p);
      errno = EPROTO;
      return NULL;
    }

  if (!mongo_wire_reply_packet_get_header (p, &rh))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  if (rh.flags & MONGO_REPLY_FLAG_QUERY_FAIL)
    {
      mongo_wire_packet_free (p);
      errno = ENOENT;
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
    {
      errno = ENOENT;
      return FALSE;
    }

  if (!bson_cursor_get_double (c, &d))
    {
      bson_cursor_free (c);
      errno = EINVAL;
      return FALSE;
    }
  bson_cursor_free (c);
  errno = (d == 1) ? 0 : EPROTO;
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
    {
      errno = ENOTCONN;
      return -1;
    }

  cmd = bson_new_sized (bson_size (query) + 32);
  bson_append_string (cmd, "count", coll, -1);
  if (query)
    bson_append_document (cmd, "query", query);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, db, cmd);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return -1;
    }
  bson_free (cmd);

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return -1;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  if (!_mongo_sync_check_ok (cmd))
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return -1;
    }

  c = bson_find (cmd, "n");
  if (!c)
    {
      bson_free (cmd);
      errno = ENOENT;
      return -1;
    }
  if (!bson_cursor_get_double (c, &d))
    {
      bson_free (cmd);
      bson_cursor_free (c);
      errno = EINVAL;
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
    {
      errno = ENOTCONN;
      return FALSE;
    }

  cmd = bson_new_sized (64);
  bson_append_string (cmd, "drop", coll, -1);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, db, cmd);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  if (!_mongo_sync_check_ok (cmd))
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
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
    {
      errno = ENOTCONN;
      return FALSE;
    }

  cmd = bson_new_sized (64);
  bson_append_int32 (cmd, "getlasterror", 1);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, db, cmd);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  if (!_mongo_sync_check_ok (cmd))
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }

  c = bson_find (cmd, "err");
  if (!c)
    {
      bson_free (cmd);
      errno = EPROTO;
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
  errno = EPROTO;
  return FALSE;
}

gboolean
mongo_sync_cmd_reset_error (mongo_sync_connection *conn,
			    const gchar *db)
{
  mongo_packet *p;
  bson *cmd;

  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }

  cmd = bson_new_sized (32);
  bson_append_int32 (cmd, "reseterror", 1);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, db, cmd);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  if (!_mongo_sync_check_ok (cmd))
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);
  return TRUE;
}

gboolean
mongo_sync_cmd_is_master (mongo_sync_connection *conn)
{
  bson *cmd, *res, *hosts;
  mongo_packet *p;
  bson_cursor *c;
  gboolean b;
  GList *l;

  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }

  cmd = bson_new_sized (32);
  bson_append_int32 (cmd, "ismaster", 1);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, "system", cmd);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &res))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (res);

  c = bson_find (res, "ismaster");
  if (!bson_cursor_get_boolean (c, &b))
    {
      bson_cursor_free (c);
      bson_free (res);
      errno = EPROTO;
      return FALSE;
    }
  bson_cursor_free (c);

  if (!b)
    {
      const gchar *s;

      /* We're not the master, so we should have a 'primary' key in
	 the response. */
      c = bson_find (res, "primary");
      if (bson_cursor_get_string (c, &s))
	{
	  g_free (conn->rs.primary);
	  conn->rs.primary = g_strdup (s);
	}
      bson_cursor_free (c);
    }

  /* Find all the members of the set, and cache them. */
  c = bson_find (res, "hosts");
  if (!c)
    {
      bson_free (res);
      errno = 0;
      return b;
    }

  if (!bson_cursor_get_array (c, &hosts))
    {
      bson_cursor_free (c);
      bson_free (res);
      errno = 0;
      return b;
    }
  bson_cursor_free (c);
  bson_free (res);
  bson_finish (hosts);

  /* Delete the old host list. */
  l = conn->rs.hosts;
  while (l)
    {
      g_free (l->data);
      l = g_list_delete_link (l, l);
    }
  conn->rs.hosts = NULL;

  c = bson_cursor_new (hosts);
  while (bson_cursor_next (c))
    {
      const gchar *s;

      if (bson_cursor_get_string (c, &s))
	conn->rs.hosts = g_list_append (conn->rs.hosts, g_strdup (s));
    }
  bson_cursor_free (c);
  bson_free (hosts);

  errno = 0;
  return b;
}

gboolean
mongo_sync_cmd_ping (mongo_sync_connection *conn)
{
  bson *cmd;
  mongo_packet *p;

  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }

  cmd = bson_new_sized (32);
  bson_append_int32 (cmd, "ping", 1);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, "system", cmd);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &cmd))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (cmd);

  if (!_mongo_sync_check_ok (cmd))
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);
  return TRUE;
}
