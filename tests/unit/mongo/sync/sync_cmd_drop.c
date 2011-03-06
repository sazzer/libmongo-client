#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_drop (void)
{
  mongo_sync_connection *c;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);

  ok (mongo_sync_cmd_drop (NULL, config.db, config.coll) == FALSE,
      "mongo_sync_cmd_drop() fails with a NULL connection");
  ok (mongo_sync_cmd_drop (c, NULL, config.coll) == FALSE,
      "mongo_sync_cmd_drop() fails with a NULL db");

  ok (mongo_sync_cmd_drop (c, config.db, config.coll) == FALSE,
      "mongo_sync_cmd_drop() fails with a bogus FD");

  mongo_sync_disconnect (c);
  test_env_free ();
}

RUN_TEST (3, mongo_sync_cmd_drop);
