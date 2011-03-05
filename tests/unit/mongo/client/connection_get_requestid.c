#include "test.h"
#include "tap.h"
#include "mongo-client.h"

void
test_mongo_connection_get_requestid (void)
{
  ok (mongo_connection_get_requestid (NULL) == -1,
      "mongo_connection_get_requestid fails with a NULL connection");
}

RUN_TEST (1, mongo_connection_get_requestid);
