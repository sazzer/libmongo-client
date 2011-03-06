#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_query (void)
{
  mongo_sync_connection *c;
  bson *q, *s;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);
  q = test_bson_generate_full ();
  s = test_bson_generate_full ();

  ok (mongo_sync_cmd_query (NULL, config.ns, 0, 0, 1, q, s) == NULL,
      "mongo_sync_cmd_query() fails with a NULL connection");
  ok (mongo_sync_cmd_query (c, NULL, 0, 0, 1, q, s) == NULL,
      "mongo_sync_cmd_query() fails with a NULL namespace");
  ok (mongo_sync_cmd_query (c, config.ns, 0, 0, 1, NULL, s) == NULL,
      "mongo_sync_cmd_query() fails with a NULL query");

  ok (mongo_sync_cmd_query (c, config.ns, 0, 0, 1, q, s) == NULL,
      "mongo_sync_cmd_query() fails with a bogus FD");
  mongo_sync_conn_set_slaveok (c, TRUE);
  ok (mongo_sync_cmd_query (c, config.ns, 0, 0, 1, q, s) == NULL,
      "mongo_sync_cmd_query() fails with a bogus FD");

  mongo_sync_disconnect (c);

  bson_free (q);
  bson_free (s);
  test_env_free ();
}

RUN_TEST (5, mongo_sync_cmd_query);
