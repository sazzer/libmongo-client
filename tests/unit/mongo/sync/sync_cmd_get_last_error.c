#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_cmd_get_last_error (void)
{
  mongo_sync_connection *c;
  gchar *error;

  test_env_setup ();

  c = test_make_fake_sync_conn (-1, FALSE);

  errno = 0;
  ok (mongo_sync_cmd_get_last_error (NULL, config.db, &error) == FALSE,
      "mongo_sync_cmd_get_last_error() returns FALSE with a NULL connection");
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is set to ENOTCONN");

  ok (mongo_sync_cmd_get_last_error (c, NULL, &error) == FALSE,
      "mongo_sync_cmd_get_last_error() fails with a NULL db");

  errno = 0;
  ok (mongo_sync_cmd_get_last_error (c, config.db, NULL) == FALSE,
      "mongo_sync_cmd_get_last_error() fails with a NULL error destination");
  cmp_ok (errno, "==", EINVAL,
	  "errno is set to EINVAL");

  mongo_sync_disconnect (c);
  test_env_free ();
}

RUN_TEST (5, mongo_sync_cmd_get_last_error);
