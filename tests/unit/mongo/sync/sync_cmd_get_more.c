#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_get_more (void)
{
  mongo_sync_connection *c;

  c = test_make_fake_sync_conn (-1, FALSE);

  ok (mongo_sync_cmd_get_more (NULL, "test.ns", 1, 1234) == NULL,
      "mongo_sync_cmd_get_more() fails with a NULL connection");
  ok (mongo_sync_cmd_get_more (c, NULL, 1, 1234) == NULL,
      "mongo_sync_cmd_get_more() fails with a NULL namespace");

  ok (mongo_sync_cmd_get_more (c, "test.ns", 1, 1234) == NULL,
      "mongo_sync_cmd_get_more() fails with a bogus FD");
  mongo_sync_conn_set_slaveok (c, TRUE);
  ok (mongo_sync_cmd_get_more (c, "test.ns", 1, 1234) == NULL,
      "mongo_sync_cmd_get_more() fails with a bogus FD");

  mongo_sync_disconnect (c);
}

RUN_TEST (4, mongo_sync_cmd_get_more);