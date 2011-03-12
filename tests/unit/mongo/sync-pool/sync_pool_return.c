#include "test.h"
#include "mongo.h"

#include <string.h>
#include "libmongo-private.h"

void
test_mongo_sync_pool_return (void)
{
  mongo_sync_pool_connection c;
  void *pool;

  pool = g_malloc (1024);

  ok (mongo_sync_pool_return (NULL, &c) == FALSE,
      "mongo_sync_pool_return() should fail without a pool");
  ok (mongo_sync_pool_return ((mongo_sync_pool *)pool, NULL) == FALSE,
      "mongo_sync_pool_return() should fail without a connection");
  g_free (pool);
}

RUN_TEST (2, mongo_sync_pool_return);
