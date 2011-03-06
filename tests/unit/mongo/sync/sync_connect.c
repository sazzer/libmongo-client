#include "test.h"
#include "mongo.h"

void
test_mongo_sync_connect (void)
{
  ok (mongo_sync_connect (NULL, 27017, FALSE) == NULL,
      "mongo_sync_connect() fails with a NULL host");
}

RUN_TEST (1, mongo_sync_connect);
