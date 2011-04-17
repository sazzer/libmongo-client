#include "test.h"
#include "mongo.h"

void
test_mongo_sync_conn_seed_add (void)
{
  mongo_sync_connection *c;

  c = test_make_fake_sync_conn (42, TRUE);

  ok (mongo_sync_conn_seed_add (NULL, "localhost", 27017) == FALSE,
      "mongo_sync_conn_seed_add() should fail with a NULL connection");
  ok (mongo_sync_conn_seed_add (c, NULL, 27017) == FALSE,
      "mongo_sync_conn_seed_add() should fail with a NULL host");
  ok (mongo_sync_conn_seed_add (c, "localhost", -1) == FALSE,
      "mongo_sync_conn_seed_add() should fail with an invalid port");

  ok (mongo_sync_conn_seed_add (c, "localhost", 27017),
      "mongo_sync_conn_seed_add() works");

  mongo_sync_disconnect (c);
}

RUN_TEST (4, mongo_sync_conn_seed_add);
