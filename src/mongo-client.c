#include "mongo-client.h"
#include "bson.h"
#include "mongo_wire.h"

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

gint
mongo_connect (const char *host, int port)
{
  struct sockaddr_in sa;
  socklen_t addrsize;
  int fd;

  memset (&sa, 0, sizeof (sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  sa.sin_addr.s_addr = inet_addr (host);
  addrsize = sizeof (sa);

  fd = socket (AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return -1;

  if (connect (fd, (struct sockaddr *)&sa, addrsize))
    {
      close (fd);
      return -1;
    }

  setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof (one));

  if (unset_nonblock (fd))
    {
      close (fd);
      return -1;
    }

  return fd;
}

void
mongo_disconnect (gint fd)
{
  if (fd < 0)
    return;
  close (fd);
}

gboolean
mongo_packet_send (gint fd, const mongo_packet *p)
{
  const guint8 *hdr, *data;
  gint32 hdr_size, data_size;

  if (fd < 0 || !p)
    return FALSE;

  hdr_size = mongo_wire_packet_get_header (p, &hdr);
  data_size = mongo_wire_packet_get_data (p, &data);

  if (hdr_size == -1 || data_size == -1)
    return FALSE;

  if (send (fd, hdr, hdr_size, 0) != hdr_size)
    return FALSE;
  if (send (fd, data, data_size, 0) != data_size)
    return FALSE;

  return TRUE;
}
