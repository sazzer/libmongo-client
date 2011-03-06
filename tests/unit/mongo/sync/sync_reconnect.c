#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_reconnect (void)
{
  mongo_sync_connection *conn;

  ok (mongo_sync_reconnect (NULL, FALSE) == NULL,
      "mongo_sync_reconnect() fails with a NULL connection");
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is ENOTCONN");

  conn = test_make_fake_sync_conn (-1, FALSE);
  ok (mongo_sync_reconnect (conn, FALSE) == NULL,
      "mongo_sync_reconnect() fails with a bogus FD");
  cmp_ok (errno, "==", EHOSTUNREACH,
	  "errno is EHOSTUNREACH");

  mongo_sync_disconnect (conn);
}

RUN_TEST (4, mongo_sync_reconnect);
