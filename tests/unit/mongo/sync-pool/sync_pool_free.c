#include "test.h"
#include "mongo.h"

void
test_mongo_sync_pool_free (void)
{
  mongo_sync_pool_free (NULL);
  pass ("mongo_sync_pool_free(NULL) works");
}

RUN_TEST (1, mongo_sync_pool_free);
