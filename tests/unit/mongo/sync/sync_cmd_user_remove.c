#include "test.h"
#include "mongo.h"
#include "config.h"

#include <errno.h>

#if ENABLE_AUTH
void
test_mongo_sync_cmd_user_remove (void)
{
  mongo_sync_connection *c;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);

  errno = 0;
  ok (mongo_sync_cmd_user_remove (NULL, config.db, "test") == FALSE,
      "mongo_sync_cmd_user_remove() fails with a NULL connection");
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is set to ENOTCONN");

  errno = 0;
  ok (mongo_sync_cmd_user_remove (c, NULL, "test") == FALSE,
      "mongo_sync_cmd_user_remove() fails with a NULL db");
  cmp_ok (errno, "==", EINVAL,
	  "errno is set to EINVAL");

  errno = 0;
  ok (mongo_sync_cmd_user_remove (c, config.db, NULL) == FALSE,
      "mongo_sync_cmd_user_remove() fails with a NULL user");
  cmp_ok (errno, "==", EINVAL,
	  "errno is set to EINVAL");

  ok (mongo_sync_cmd_user_remove (c, config.db, "test") == FALSE,
      "mongo_sync_cmd_user_remove() fails with a bogus FD");

  mongo_sync_disconnect (c);
  test_env_free ();
}

RUN_TEST (7, mongo_sync_cmd_user_remove);
#else
void
test_mongo_sync_cmd_user_remove  (void)
{
  errno = 0;
  if (mongo_sync_cmd_user_remove (NULL, NULL, NULL) != FALSE)
    fail ("mongo_sync_user_remove() with NULLs should fail");
  else
    ok (errno == ENOTSUP,
	"mongo_sync_user_remove() should fail with ENOTSUP when "
	"authentication is not compiled in");
}

RUN_TEST (1, mongo_sync_cmd_user_remove);
#endif
