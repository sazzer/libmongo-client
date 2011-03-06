#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_cmd_ping (void)
{
  mongo_sync_connection *c;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);

  errno = 0;
  ok (mongo_sync_cmd_ping (NULL) == FALSE,
      "mongo_sync_cmd_ping(NULL) returns FALSE");
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is set to ENOTCONN");

  errno = 0;
  ok (mongo_sync_cmd_ping (c) == FALSE,
      "Pinging a bogus connection fails");
  cmp_ok (errno, "!=", 0,
	  "errno is not 0");

  mongo_sync_disconnect (c);
  test_env_free ();
}

RUN_TEST (4, mongo_sync_cmd_ping);
