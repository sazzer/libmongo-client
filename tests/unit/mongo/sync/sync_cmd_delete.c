#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_delete (void)
{
  mongo_sync_connection *c;
  bson *b;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);
  b = test_bson_generate_full ();

  ok (mongo_sync_cmd_delete (NULL, config.ns, 0, b) == FALSE,
      "mongo_sync_cmd_delete() fails with a NULL connection");
  ok (mongo_sync_cmd_delete (c, NULL, 0, b) == FALSE,
      "mongo_sync_cmd_delete() fails with a NULL namespace");
  ok (mongo_sync_cmd_delete (c, config.ns, 0, NULL) == FALSE,
      "mongo_sync_cmd_delete() fails with a NULL selector");

  ok (mongo_sync_cmd_delete (c, config.ns, 0, b) == FALSE,
      "mongo_sync_cmd_delete() fails with a bogus FD");

  bson_free (b);
  mongo_sync_disconnect (c);
  test_env_free ();
}

RUN_TEST (4, mongo_sync_cmd_delete);
