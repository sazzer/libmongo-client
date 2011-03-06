#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_insert (void)
{
  mongo_sync_connection *c;
  bson *b1, *b2;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);
  b1 = test_bson_generate_full ();
  b2 = test_bson_generate_full ();

  ok (mongo_sync_cmd_insert (NULL, config.ns, b1, b2, NULL) == FALSE,
      "mongo_sync_cmd_insert() fails with a NULL connection");
  ok (mongo_sync_cmd_insert (c, NULL, b1, b2, NULL) == FALSE,
      "mongo_sync_cmd_insert() fails with a NULL namespace");
  ok (mongo_sync_cmd_insert (c, config.ns, NULL) == FALSE,
      "mongo_sync_cmd_insert() fails with no documents to insert");
  ok (mongo_sync_cmd_insert (c, config.ns, b1, b2, NULL) == FALSE,
      "mongo_sync_cmd_insert() fails with a bogus FD");

  mongo_sync_disconnect (c);
  bson_free (b1);
  bson_free (b2);

  test_env_free ();
}

RUN_TEST (4, mongo_sync_cmd_insert);
