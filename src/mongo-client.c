/* mongo-client.c - libmongo-client user API
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

#include "mongo-client.h"
#include "bson.h"
#include "mongo-wire.h"
#include "libmongo-private.h"

#include <glib.h>

#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

static const int one = 1;

static int
unset_nonblock (int fd)
{
  int val;

  val = fcntl (fd, F_GETFL, 0);
  if (val < 0)
    return -1;

  if (!(val & O_NONBLOCK))
    return 0;

  val &= ~O_NONBLOCK;
  if (fcntl (fd, F_SETFL, val) == -1)
    return -1;

  return 0;
}

mongo_connection *
mongo_connect (const char *host, int port)
{
  struct addrinfo *res = NULL, *r;
  struct addrinfo hints;
  int e, fd = -1;
  gchar *port_s;
  mongo_connection *conn;

  if (!host)
    {
      errno = EINVAL;
      return NULL;
    }

  conn = g_try_new0 (mongo_connection, 1);
  if (!conn)
    {
      errno = ENOMEM;
      return NULL;
    }

  memset (&hints, 0, sizeof (hints));
  hints.ai_socktype = SOCK_STREAM;

#ifdef __linux__
  hints.ai_flags = AI_ADDRCONFIG;
#endif

  port_s = g_strdup_printf ("%d", port);
  e = getaddrinfo (host, port_s, &hints, &res);
  if (e != 0)
    {
      int err = errno;

      g_free (conn);
      g_free (port_s);
      errno = err;
      return NULL;
    }
  g_free (port_s);

  for (r = res; r != NULL; r = r->ai_next)
    {
      fd = socket (r->ai_family, r->ai_socktype, r->ai_protocol);
      if (fd != -1 && connect (fd, r->ai_addr, r->ai_addrlen) == 0)
	break;
      if (fd != -1)
	{
	  close (fd);
	  fd = -1;
	}
    }
  freeaddrinfo (res);

  if (fd == -1)
    {
      g_free (conn);

      errno = EADDRNOTAVAIL;
      return NULL;
    }

  setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof (one));

  if (unset_nonblock (fd))
    {
      int err = errno;

      g_free (conn);
      close (fd);
      errno = err;
      return NULL;
    }

  conn->fd = fd;

  return conn;
}

void
mongo_disconnect (mongo_connection *conn)
{
  if (!conn)
    {
      errno = ENOTCONN;
      return;
    }

  if (conn->fd >= 0)
    close (conn->fd);

  g_free (conn);
  errno = 0;
}

gboolean
mongo_packet_send (mongo_connection *conn, const mongo_packet *p)
{
  const guint8 *data;
  gint32 data_size;
  mongo_packet_header h;
  struct iovec iov[2];
  struct msghdr msg;

  if (!conn)
    {
      errno = ENOTCONN;
      return FALSE;
    }
  if (!p)
    {
      errno = EINVAL;
      return FALSE;
    }

  if (conn->fd < 0)
    {
      errno = EBADF;
      return FALSE;
    }

  if (!mongo_wire_packet_get_header_raw (p, &h))
    return FALSE;

  data_size = mongo_wire_packet_get_data (p, &data);

  if (data_size == -1)
    return FALSE;

  iov[0].iov_base = (void *)&h;
  iov[0].iov_len = sizeof (h);
  iov[1].iov_base = (void *)data;
  iov[1].iov_len = data_size;

  memset (&msg, 0, sizeof (struct msghdr));
  msg.msg_iov = iov;
  msg.msg_iovlen = 2;

  if (sendmsg (conn->fd, &msg, MSG_NOSIGNAL) != (gint32)sizeof (h) + data_size)
    return FALSE;

  conn->request_id = h.id;

  return TRUE;
}

mongo_packet *
mongo_packet_recv (mongo_connection *conn)
{
  mongo_packet *p;
  guint8 *data;
  guint32 size;
  mongo_packet_header h;

  if (!conn)
    {
      errno = ENOTCONN;
      return NULL;
    }

  if (conn->fd < 0)
    {
      errno = EBADF;
      return NULL;
    }

  memset (&h, 0, sizeof (h));
  if (recv (conn->fd, &h, sizeof (mongo_packet_header), MSG_NOSIGNAL) !=
      sizeof (mongo_packet_header))
    {
      return NULL;
    }

  h.length = GINT32_FROM_LE (h.length);
  h.id = GINT32_FROM_LE (h.id);
  h.resp_to = GINT32_FROM_LE (h.resp_to);
  h.opcode = GINT32_FROM_LE (h.opcode);

  p = mongo_wire_packet_new ();

  if (!mongo_wire_packet_set_header_raw (p, &h))
    {
      int e = errno;

      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  size = h.length - sizeof (mongo_packet_header);
  data = g_try_new0 (guint8, size);
  if (recv (conn->fd, data, size, MSG_NOSIGNAL) != size)
    {
      int e = errno;

      g_free (data);
      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  if (!mongo_wire_packet_set_data (p, data, size))
    {
      int e = errno;

      g_free (data);
      mongo_wire_packet_free (p);
      errno = e;
      return NULL;
    }

  g_free (data);

  return p;
}

gint32
mongo_connection_get_requestid (const mongo_connection *conn)
{
  if (!conn)
    {
      errno = ENOTCONN;
      return -1;
    }

  return conn->request_id;
}
