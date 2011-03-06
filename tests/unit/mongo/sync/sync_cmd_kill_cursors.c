#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_kill_cursors (void)
{
  mongo_sync_connection *c;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);

  ok (mongo_sync_cmd_kill_cursors (NULL, 1, (gint64)1234) == FALSE,
      "mongo_sync_cmd_kill_cursors() fails with a NULL connection");
  ok (mongo_sync_cmd_kill_cursors (c, 0, (gint64)1234) == FALSE,
      "mongo_sync_cmd_kill_cursors() fails with a negative number of cursors");

  ok (mongo_sync_cmd_kill_cursors (c, 1, (gint64)1234) == FALSE,
      "mongo_sync_cmd_kill_cursors() fails with a bogus FD");

  mongo_sync_disconnect (c);
  test_env_free ();
}

RUN_TEST (3, mongo_sync_cmd_kill_cursors);
