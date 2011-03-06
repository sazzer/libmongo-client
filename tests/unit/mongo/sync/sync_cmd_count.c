#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_count (void)
{
  mongo_sync_connection *c;
  bson *b;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);
  b = test_bson_generate_full ();

  ok (mongo_sync_cmd_count (NULL, config.db, config.coll, b) == -1,
      "mongo_sync_cmd_count() fails with a NULL connection");
  ok (mongo_sync_cmd_count (c, NULL, config.coll, b) == -1,
      "mongo_sync_cmd_count() fails with a NULL db");
  ok (mongo_sync_cmd_count (c, config.db, NULL, b) == -1,
      "mongo_sync_cmd_count() fails with a NULL collection");

  ok (mongo_sync_cmd_count (c, config.db, config.coll, b) == -1,
      "mongo_sync_cmd_count() fails with a bogus FD");
  mongo_sync_conn_set_slaveok (c, TRUE);
  ok (mongo_sync_cmd_count (c, config.db, config.coll, b) == -1,
      "mongo_sync_cmd_count() fails with a bogus FD");

  bson_free (b);
  mongo_sync_disconnect (c);

  test_env_free ();
}

RUN_TEST (5, mongo_sync_cmd_count);
