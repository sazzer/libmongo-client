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

#include <glib.h>

#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>

static const int one = 1;

/** @internal Mongo Connection state object. */
struct _mongo_connection
{
  gint fd; /**< The file descriptor associated with the connection. */
};

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
  struct sockaddr_in sa;
  socklen_t addrsize;
  int fd;
  mongo_connection *c;

  memset (&sa, 0, sizeof (sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  sa.sin_addr.s_addr = inet_addr (host);
  addrsize = sizeof (sa);

  fd = socket (AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return NULL;

  if (connect (fd, (struct sockaddr *)&sa, addrsize))
    {
      close (fd);
      return NULL;
    }

  setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof (one));

  if (unset_nonblock (fd))
    {
      close (fd);
      return NULL;
    }

  c = (mongo_connection *)g_try_new0 (mongo_connection, 1);
  if (!c)
    return NULL;

  c->fd = fd;

  return c;
}

void
mongo_disconnect (mongo_connection *conn)
{
  if (!conn)
    return;

  if (conn->fd >= 0)
    close (conn->fd);

  g_free (conn);
}

gboolean
mongo_packet_send (mongo_connection *conn, const mongo_packet *p)
{
  const guint8 *data;
  gint32 data_size;
  mongo_packet_header h;

  if (!conn || !p)
    return FALSE;
  if (conn->fd < 0)
    return FALSE;

  if (!mongo_wire_packet_get_header (p, &h))
    return FALSE;

  data_size = mongo_wire_packet_get_data (p, &data);

  if (data_size == -1)
    return FALSE;

  if (send (conn->fd, &h, sizeof (h), 0) != sizeof (h))
    return FALSE;
  if (send (conn->fd, data, data_size, 0) != data_size)
    return FALSE;

  return TRUE;
}

mongo_packet *
mongo_packet_recv (mongo_connection *conn)
{
  mongo_packet *p;
  guint8 *data;
  guint32 size;
  mongo_packet_header h;

  if (!conn || conn->fd < 0)
    return NULL;

  memset (&h, 0, sizeof (h));
  if (recv (conn->fd, &h, sizeof (mongo_packet_header), 0) !=
      sizeof (mongo_packet_header))
    {
      return NULL;
    }

  h.length = GINT32_FROM_LE (h.length);
  h.id = GINT32_FROM_LE (h.id);
  h.resp_to = GINT32_FROM_LE (h.resp_to);
  h.opcode = GINT32_FROM_LE (h.opcode);

  p = mongo_wire_packet_new ();

  if (!mongo_wire_packet_set_header (p, &h))
    {
      mongo_wire_packet_free (p);
      return NULL;
    }

  size = h.length - sizeof (mongo_packet_header);
  data = g_try_new0 (guint8, size);
  if (recv (conn->fd, data, size, 0) != size)
    {
      g_free (data);
      mongo_wire_packet_free (p);
      return NULL;
    }

  if (!mongo_wire_packet_set_data (p, data, size))
    {
      g_free (data);
      mongo_wire_packet_free (p);
      return NULL;
    }

  g_free (data);

  return p;
}
