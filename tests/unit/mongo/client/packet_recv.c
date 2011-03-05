#include "test.h"
#include "tap.h"
#include "mongo-client.h"

#include <errno.h>
#include "libmongo-private.h"

void
test_mongo_packet_recv (void)
{
  mongo_connection conn;

  conn.fd = -1;

  ok (mongo_packet_recv (NULL) == NULL,
      "mongo_packet_recv() fails with a NULL connection");
  ok (errno == ENOTCONN,
      "mongo_packet_recv() sets errno to ENOTCONN if connection is NULL");

  ok (mongo_packet_recv (&conn) == NULL,
      "mongo_packet_recv() fails if the FD is less than zero");
  ok (errno == EBADF,
      "mongo_packet_recv() sets errno to EBADF is the FD is bad");
}

RUN_TEST (4, mongo_packet_recv);
