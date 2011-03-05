#include "test.h"
#include "tap.h"
#include "mongo-client.h"

#include "libmongo-private.h"
#include <errno.h>

void
test_mongo_disconnect (void)
{
  mongo_connection *conn;

  conn = g_new0 (mongo_connection, 1);
  conn->fd = -1;

  errno = 0;
  mongo_disconnect (NULL);
  ok (errno == ENOTCONN,
      "mongo_disconnect() fails with ENOTCONN when passed a NULL connection");

  mongo_disconnect (conn);
  ok (errno == 0,
      "mongo_disconnect() works");

  conn = g_new0 (mongo_connection, 1);
  conn->fd = 100;
  mongo_disconnect (conn);
  ok (errno == 0,
      "mongo_disconnect() works, even with a bogus FD");
}

RUN_TEST (3, mongo_disconnect);
