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

#include "config.h"
#include "mongo.h"
#include "libmongo-private.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#if ENABLE_AUTH
#include <openssl/md5.h>
#endif

mongo_sync_connection *
mongo_sync_connect (const gchar *host, int port,
		    gboolean slaveok)
{
  mongo_sync_connection *s;
  mongo_connection *c;

  c = mongo_connect (host, port);
  if (!c)
    return NULL;
  s = g_try_realloc (c, sizeof (mongo_sync_connection));
  if (!s)
    return NULL;

  s->slaveok = slaveok;
  s->rs.hosts = NULL;
  s->rs.primary = NULL;
  s->last_error = NULL;
  s->max_insert_size = MONGO_SYNC_DEFAULT_MAX_INSERT_SIZE;

  return s;
}

static void
_mongo_sync_connect_replace (mongo_sync_connection *old,
			     mongo_sync_connection *new)
{
  GList *l;

  if (!old || !new)
    return;

  g_free (old->rs.primary);

  /* Delete the host list. */
  l = old->rs.hosts;
  while (l)
    {
      g_free (l->data);
      l = g_list_delete_link (l, l);
    }

  if (old->super.fd)
    close (old->super.fd);

  old->super.fd = new->super.fd;
  old->super.request_id = -1;
  old->slaveok = new->slaveok;
  old->rs.primary = NULL;
  old->rs.hosts = NULL;
  g_free (old->last_error);
  old->last_error = NULL;

  /* Free the replicaset struct in the new connection. These aren't
     copied, in order to avoid infinite loops. */
  l = new->rs.hosts;
  while (l)
    {
      g_free (l->data);
      l = g_list_delete_link (l, l);
    }
  g_free (new->rs.primary);
  g_free (new->last_error);
  g_free (new);
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
	      _mongo_sync_connect_replace (conn, nc);
	      errno = e;
	      return conn;
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
      _mongo_sync_connect_replace (conn, nc);
      errno = e;
      return conn;
    }

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
  g_free (conn->last_error);

  /* Delete the host list. */
  l = conn->rs.hosts;
  while (l)
    {
      g_free (l->data);
      l = g_list_delete_link (l, l);
    }

  mongo_disconnect ((mongo_connection *)conn);
}

gint32
mongo_sync_conn_get_max_insert_size (mongo_sync_connection *conn)
{
  if (!conn)
    {
      errno = ENOTCONN;
      return -1;
    }
  return conn->max_insert_size;
}

void
mongo_sync_conn_set_max_insert_size (mongo_sync_connection *conn,
				     gint32 max_size)
{
  if (!conn)
    {
      errno = ENOTCONN;
      return;
    }
  if (max_size <= 0)
    {
      errno = ERANGE;
      return;
    }

  errno = 0;
  conn->max_insert_size = max_size;
}

gboolean
mongo_sync_conn_get_slaveok (const mongo_sync_connection *conn)
{
  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }

  errno = 0;
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

static gboolean
_mongo_cmd_chk_conn (mongo_sync_connection *conn,
		     gboolean force_master)
{
  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }

  if (force_master || !conn->slaveok)
    {
      errno = 0;
      if (!mongo_sync_cmd_is_master (conn))
	{
	  if (errno == EPROTO)
	    return FALSE;
	  if (!mongo_sync_reconnect (conn, TRUE))
	    return FALSE;
	}
      return TRUE;
    }

  errno = 0;
  if (!mongo_sync_cmd_ping (conn))
    {
      if (errno == EPROTO)
	return FALSE;
      if (!mongo_sync_reconnect (conn, FALSE))
	{
	  errno = ENOTCONN;
	  return FALSE;
	}
    }
  errno = 0;
  return TRUE;
}

gboolean
mongo_sync_cmd_update (mongo_sync_connection *conn,
		       const gchar *ns,
		       gint32 flags, const bson *selector,
		       const bson *update)
{
  mongo_packet *p;
  gint32 rid;

  if (!_mongo_cmd_chk_conn (conn, TRUE))
    return FALSE;

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
mongo_sync_cmd_insert_n (mongo_sync_connection *conn,
			 const gchar *ns, gint32 n,
			 const bson **docs)
{
  mongo_packet *p;
  gint32 rid;
  gint32 pos = 0, c = n, i = 0;
  gint32 size = 0;

  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }

  if (!ns || !docs)
    {
      errno = EINVAL;
      return FALSE;
    }
  if (n <= 0)
    {
      errno = EINVAL;
      return FALSE;
    }

  for (i = 0; i < n; i++)
    {
      if (bson_size (docs[i]) >= conn->max_insert_size)
	{
	  errno = EMSGSIZE;
	  return FALSE;
	}
    }

  if (!_mongo_cmd_chk_conn (conn, TRUE))
    return FALSE;

  do
    {
      i = pos;
      c = 0;

      while (i < n && size < conn->max_insert_size)
	{
	  size += bson_size (docs[i++]);
	  c++;
	}
      size = 0;
      if (i < n)
	c--;

      rid = mongo_connection_get_requestid ((mongo_connection *)conn) + 1;

      p = mongo_wire_cmd_insert_n (rid, ns, c, &docs[pos]);
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
      pos += c;
    } while (pos < n);

  return TRUE;
}

gboolean
mongo_sync_cmd_insert (mongo_sync_connection *conn,
		       const char *ns, ...)
{
  gboolean b;
  bson **docs, *d;
  gint32 n = 0;
  va_list ap;

  if (!ns)
    {
      errno = EINVAL;
      return FALSE;
    }

  docs = (bson **)g_try_new0 (bson *, 1);
  if (!docs)
    return FALSE;

  va_start (ap, ns);
  while ((d = (bson *)va_arg (ap, gpointer)))
    {
      if (bson_size (d) < 0)
	{
	  g_free (docs);
	  errno = EINVAL;
	  return FALSE;
	}

      docs = (bson **)g_try_realloc (docs, n + 1);
      if (!docs)
	return FALSE;
      docs[n++] = d;
    }
  va_end (ap);

  b = mongo_sync_cmd_insert_n (conn, ns, n, (const bson **)docs);
  g_free (docs);
  return b;
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

  if (!_mongo_cmd_chk_conn (conn, !((conn && conn->slaveok) ||
				    (flags & MONGO_WIRE_FLAG_QUERY_SLAVE_OK))))
    return FALSE;

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

  if (!mongo_wire_packet_get_header_raw (p, &h))
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
      errno = EPROTO;
      return NULL;
    }

  if (rh.returned == 0)
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

  if (!_mongo_cmd_chk_conn (conn, FALSE))
    return FALSE;

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

  if (!mongo_wire_packet_get_header_raw (p, &h))
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
      errno = EPROTO;
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

  if (!_mongo_cmd_chk_conn (conn, TRUE))
    return FALSE;

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

  if (n <= 0)
    {
      errno = EINVAL;
      return FALSE;
    }

  if (!_mongo_cmd_chk_conn (conn, FALSE))
    return FALSE;

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

static gboolean
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

static gboolean
_mongo_sync_get_error (const bson *rep, gchar **error)
{
  bson_cursor *c;

  if (!error)
    return FALSE;

  c = bson_find (rep, "err");
  if (!c)
    {
      c = bson_find (rep, "errmsg");
      if (!c)
	{
	  errno = EPROTO;
	  return FALSE;
	}
    }
  if (bson_cursor_type (c) == BSON_TYPE_NULL)
    {
      *error = NULL;
      bson_cursor_free (c);
      return TRUE;
    }
  else if (bson_cursor_type (c) == BSON_TYPE_STRING)
    {
      const gchar *err;

      bson_cursor_get_string (c, &err);
      *error = g_strdup (err);
      bson_cursor_free (c);
      return TRUE;
    }
  errno = EPROTO;
  return FALSE;
}

static mongo_packet *
_mongo_sync_cmd_custom (mongo_sync_connection *conn,
			const gchar *db,
			const bson *command,
			gboolean check_conn,
			gboolean force_master)
{
  mongo_packet *p;
  gint32 rid;

  mongo_packet_header h;
  mongo_reply_packet_header rh;
  bson *b;

  if (!conn)
    {
      errno = ENOTCONN;
      return NULL;
    }

  if (check_conn)
    if (!_mongo_cmd_chk_conn (conn, force_master))
      return NULL;

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

  if (!mongo_wire_packet_get_header_raw (p, &h))
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

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &b))
    {
      mongo_wire_packet_free (p);
      errno = EPROTO;
      return NULL;
    }
  bson_finish (b);

  if (!_mongo_sync_check_ok (b))
    {
      int e = errno;

      g_free (conn->last_error);
      conn->last_error = NULL;
      _mongo_sync_get_error (b, &conn->last_error);
      bson_free (b);
      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }
  bson_free (b);

  return p;
}

mongo_packet *
mongo_sync_cmd_custom (mongo_sync_connection *conn,
		       const gchar *db,
		       const bson *command)
{
  return _mongo_sync_cmd_custom (conn, db, command, TRUE, FALSE);
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

  cmd = bson_new_sized (bson_size (query) + 32);
  bson_append_string (cmd, "count", coll, -1);
  if (query)
    bson_append_document (cmd, "query", query);
  bson_finish (cmd);

  p = _mongo_sync_cmd_custom (conn, db, cmd, TRUE, FALSE);
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

  cmd = bson_new_sized (64);
  bson_append_string (cmd, "drop", coll, -1);
  bson_finish (cmd);

  p = _mongo_sync_cmd_custom (conn, db, cmd, TRUE, TRUE);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);
  mongo_wire_packet_free (p);

  return TRUE;
}

gboolean
mongo_sync_cmd_get_last_error (mongo_sync_connection *conn,
			       const gchar *db, gchar **error)
{
  mongo_packet *p;
  bson *cmd;

  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }
  if (!error)
    {
      errno = EINVAL;
      return FALSE;
    }

  cmd = bson_new_sized (64);
  bson_append_int32 (cmd, "getlasterror", 1);
  bson_finish (cmd);

  p = _mongo_sync_cmd_custom (conn, db, cmd, FALSE, FALSE);
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

  if (!_mongo_sync_get_error (cmd, error))
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);

  if (*error == NULL)
    *error = g_strdup (conn->last_error);
  else
    {
      g_free (conn->last_error);
      conn->last_error = NULL;
    }

  return TRUE;
}

gboolean
mongo_sync_cmd_reset_error (mongo_sync_connection *conn,
			    const gchar *db)
{
  mongo_packet *p;
  bson *cmd;

  if (!_mongo_cmd_chk_conn (conn, FALSE))
    {
      int e = errno;

      if (conn)
	{
	  g_free (conn->last_error);
	  conn->last_error = NULL;
	}

      errno = e;
      return FALSE;
    }

  cmd = bson_new_sized (32);
  bson_append_int32 (cmd, "reseterror", 1);
  bson_finish (cmd);

  p = _mongo_sync_cmd_custom (conn, db, cmd, FALSE, FALSE);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);
  mongo_wire_packet_free (p);
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

  cmd = bson_new_sized (32);
  bson_append_int32 (cmd, "ismaster", 1);
  bson_finish (cmd);

  p = _mongo_sync_cmd_custom (conn, "system", cmd, FALSE, FALSE);
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

  cmd = bson_new_sized (32);
  bson_append_int32 (cmd, "ping", 1);
  bson_finish (cmd);

  p = _mongo_sync_cmd_custom (conn, "system", cmd, FALSE, FALSE);
  if (!p)
    {
      int e = errno;

      bson_free (cmd);
      errno = e;
      return FALSE;
    }
  bson_free (cmd);
  mongo_wire_packet_free (p);

  errno = 0;
  return TRUE;
}

#if ENABLE_AUTH
static void
digest2hex (guint8 digest[16], guint8 hex_digest[33])
{
  static const char hex[16] =
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
     'a', 'b', 'c', 'd', 'e', 'f'};
  int i;

  for (i = 0; i < 16; i++)
    {
      hex_digest[2 * i] = hex[(digest[i] & 0xf0) >> 4];
      hex_digest[2 * i + 1] = hex[digest[i] & 0x0f];
    }
  hex_digest[32] = '\0';
}

static void _pass_digest (const gchar *user, const gchar *pw,
			  guint8 hex_digest[33])
{
  MD5_CTX mc;
  guint8 digest[16];

  MD5_Init (&mc);
  MD5_Update (&mc, (const void *)user, strlen (user));
  MD5_Update (&mc, (const void *)":mongo:", 7);
  MD5_Update (&mc, (const void *)pw, strlen (pw));
  MD5_Final (digest, &mc);
  digest2hex (digest, hex_digest);
}

gboolean
mongo_sync_cmd_user_add (mongo_sync_connection *conn,
			 const gchar *db,
			 const gchar *user,
			 const gchar *pw)
{
  bson *s, *u;
  gchar *userns;
  guint8 hex_digest[33];

  if (!db || !user || !pw)
    {
      errno = EINVAL;
      return FALSE;
    }

  if (!_mongo_cmd_chk_conn (conn, TRUE))
    return FALSE;

  userns = g_strconcat (db, ".system.users", NULL);
  if (!userns)
    return FALSE;

  _pass_digest (user, pw, hex_digest);

  s = bson_build (BSON_TYPE_STRING, "user", user, -1,
		  BSON_TYPE_NONE);
  bson_finish (s);
  u = bson_build_full (BSON_TYPE_DOCUMENT, "$set", TRUE,
		       bson_build (BSON_TYPE_STRING, "pwd", hex_digest, -1,
				   BSON_TYPE_NONE),
		       BSON_TYPE_NONE);
  bson_finish (u);

  if (!mongo_sync_cmd_update (conn, userns, MONGO_WIRE_FLAG_UPDATE_UPSERT,
			      s, u))
    {
      int e = errno;

      bson_free (s);
      bson_free (u);
      g_free (userns);
      errno = e;
      return FALSE;
    }
  bson_free (s);
  bson_free (u);
  g_free (userns);

  return TRUE;
}

gboolean
mongo_sync_cmd_user_remove (mongo_sync_connection *conn,
			    const gchar *db,
			    const gchar *user)
{
  bson *s;
  gchar *userns;

  if (!db || !user)
    {
      errno = EINVAL;
      return FALSE;
    }

  if (!_mongo_cmd_chk_conn (conn, TRUE))
    return FALSE;

  userns = g_strconcat (db, ".system.users", NULL);
  if (!userns)
    return FALSE;

  s = bson_build (BSON_TYPE_STRING, "user", user, -1,
		  BSON_TYPE_NONE);
  bson_finish (s);

  if (!mongo_sync_cmd_delete (conn, userns, 0, s))
    {
      int e = errno;

      bson_free (s);
      g_free (userns);
      errno = e;
      return FALSE;
    }
  bson_free (s);
  g_free (userns);

  return TRUE;
}

gboolean
mongo_sync_cmd_authenticate (mongo_sync_connection *conn,
			     const gchar *db,
			     const gchar *user,
			     const gchar *pw)
{
  bson *b;
  mongo_packet *p;
  const gchar *s;
  gchar *nonce;
  bson_cursor *c;

  MD5_CTX mc;
  guint8 digest[16];
  guint8 hex_digest[33];

  if (!db || !user || !pw)
    {
      errno = EINVAL;
      return FALSE;
    }

  /* Obtain nonce */
  b = bson_new_sized (32);
  bson_append_int32 (b, "getnonce", 1);
  bson_finish (b);

  p = mongo_sync_cmd_custom (conn, db, b);
  if (!p)
    {
      int e = errno;

      bson_free (b);
      errno = e;
      return FALSE;
    }
  bson_free (b);

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &b))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return FALSE;
    }
  mongo_wire_packet_free (p);
  bson_finish (b);

  c = bson_find (b, "nonce");
  if (!c)
    {
      bson_free (b);
      errno = EPROTO;
      return FALSE;
    }
  if (!bson_cursor_get_string (c, &s))
    {
      bson_free (b);
      errno = EPROTO;
      return FALSE;
    }
  nonce = g_strdup (s);
  bson_cursor_free (c);
  bson_free (b);

  /* Generate the password digest. */
  _pass_digest (user, pw, hex_digest);

  /* Generate the key */
  MD5_Init (&mc);
  MD5_Update (&mc, (const void *)nonce, strlen (nonce));
  MD5_Update (&mc, (const void *)user, strlen (user));
  MD5_Update (&mc, (const void *)hex_digest, 32);
  MD5_Final (digest, &mc);
  digest2hex (digest, hex_digest);

  /* Run the authenticate command. */
  b = bson_build (BSON_TYPE_INT32, "authenticate", 1,
		  BSON_TYPE_STRING, "user", user, -1,
		  BSON_TYPE_STRING, "nonce", nonce, -1,
		  BSON_TYPE_STRING, "key", hex_digest, -1,
		  BSON_TYPE_NONE);
  bson_finish (b);
  g_free (nonce);

  p = mongo_sync_cmd_custom (conn, db, b);
  if (!p)
    {
      int e = errno;

      bson_free (b);
      errno = e;
      return FALSE;
    }
  bson_free (b);
  mongo_wire_packet_free (p);

  return TRUE;
}
#else
gboolean
mongo_sync_cmd_user_add (mongo_sync_connection *conn,
			 const gchar *db,
			 const gchar *user,
			 const gchar *pw)
{
  errno = ENOTSUP;
  return FALSE;
}

gboolean
mongo_sync_cmd_user_remove (mongo_sync_connection *conn,
			    const gchar *db,
			    const gchar *user)
{
  errno = ENOTSUP;
  return FALSE;
}

gboolean
mongo_sync_cmd_authenticate (mongo_sync_connection *conn,
			     const gchar *db,
			     const gchar *user,
			     const gchar *pw)
{
  errno = ENOTSUP;
  return FALSE;
}
#endif
