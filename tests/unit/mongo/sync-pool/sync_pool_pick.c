#include "test.h"
#include "mongo.h"

void
test_mongo_sync_pool_pick (void)
{
  ok (mongo_sync_pool_pick (NULL, TRUE) == NULL,
      "mongo_sync_pool_pick() should fail without a pool");
}

RUN_TEST (1, mongo_sync_pool_pick);
