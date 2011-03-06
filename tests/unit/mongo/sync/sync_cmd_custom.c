#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_custom (void)
{
  mongo_sync_connection *c;
  bson *cmd;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);
  cmd = bson_new ();
  bson_append_int32 (cmd, "getnonce", 1);
  bson_finish (cmd);

  ok (mongo_sync_cmd_custom (NULL, config.db, cmd) == NULL,
      "mongo_sync_cmd_custom() fails with a NULL connection");
  ok (mongo_sync_cmd_custom (c, NULL, cmd) == NULL,
      "mongo_sync_cmd_custom() fails with a NULL namespace");

  ok (mongo_sync_cmd_custom (c, config.db, cmd) == NULL,
      "mongo_sync_cmd_custom() fails with a bogus FD");
  mongo_sync_conn_set_slaveok (c, TRUE);
  ok (mongo_sync_cmd_custom (c, config.db, cmd) == NULL,
      "mongo_sync_cmd_custom() fails with a bogus FD");

  bson_free (cmd);
  mongo_sync_disconnect (c);

  test_env_free ();
}

RUN_TEST (4, mongo_sync_cmd_custom);
