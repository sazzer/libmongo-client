#include "test.h"
#include "tap.h"
#include "mongo-client.h"

#include "libmongo-private.h"

void
test_mongo_connection_get_requestid (void)
{
  mongo_connection conn;

  conn.request_id = 42;

  ok (mongo_connection_get_requestid (NULL) == -1,
      "mongo_connection_get_requestid() fails with a NULL connection");
  ok (mongo_connection_get_requestid (&conn) == 42,
      "mongo_connection_get_requestid() works");
}

RUN_TEST (2, mongo_connection_get_requestid);
