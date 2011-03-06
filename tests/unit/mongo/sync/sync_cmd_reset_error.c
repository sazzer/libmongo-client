#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_cmd_reset_error (void)
{
  mongo_sync_connection *c;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);

  errno = 0;
  ok (mongo_sync_cmd_reset_error (NULL, config.db) == FALSE,
      "mongo_sync_cmd_reset_error() fails with a NULL connection");
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is set to ENOTCONN");

  ok (mongo_sync_cmd_reset_error (c, NULL) == FALSE,
      "mongo_sync_cmd_reset_error() fails with a NULL db");

  ok (mongo_sync_cmd_reset_error (c, config.db) == FALSE,
      "mongo_sync_cmd_reset_error() fails with a bogus FD");

  mongo_sync_disconnect (c);
  test_env_free ();
}

RUN_TEST (4, mongo_sync_cmd_reset_error);
