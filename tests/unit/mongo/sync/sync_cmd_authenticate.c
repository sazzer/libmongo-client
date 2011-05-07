#include "test.h"
#include "mongo.h"
#include "config.h"

#include <errno.h>
#include <sys/socket.h>
#include "libmongo-private.h"

#if ENABLE_AUTH
void
test_mongo_sync_cmd_authenticate_net_secondary (void)
{
  mongo_sync_connection *c;

  skip (!config.secondary_host, 4,
	"Secondary server not configured");

  c = mongo_sync_connect (config.secondary_host, config.secondary_port, TRUE);
  mongo_sync_conn_set_auto_reconnect (c, TRUE);
  mongo_sync_cmd_is_master (c);

  ok (mongo_sync_cmd_authenticate (c, config.db, "test", "s3kr1+") == TRUE,
      "mongo_sync_cmd_authenticate() works");
  ok (mongo_sync_cmd_authenticate (c, config.db, "test", "bad_pw") == FALSE,
      "mongo_sync_cmd_authenticate() should fail with a bad password");
  ok (mongo_sync_cmd_authenticate (c, config.db, "xxx", "s3kr1+") == FALSE,
      "mongo_sync_cmd_authenticate() should fail with a bad username");

  shutdown (c->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_authenticate (c, config.db, "test", "s3kr1+") == TRUE,
      "mongo_sync_cmd_authenticate() automatically reconnects");

  mongo_sync_disconnect (c);

  endskip;
}

void
test_mongo_sync_cmd_authenticate_net (void)
{
  mongo_sync_connection *c;

  begin_network_tests (8);

  c = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  mongo_sync_conn_set_auto_reconnect (c, TRUE);

  mongo_sync_cmd_user_add (c, config.db, "test", "s3kr1+");

  ok (mongo_sync_cmd_authenticate (c, config.db, "test", "s3kr1+") == TRUE,
      "mongo_sync_cmd_authenticate() works");
  ok (mongo_sync_cmd_authenticate (c, config.db, "test", "bad_pw") == FALSE,
      "mongo_sync_cmd_authenticate() should fail with a bad password");
  ok (mongo_sync_cmd_authenticate (c, config.db, "xxx", "s3kr1+") == FALSE,
      "mongo_sync_cmd_authenticate() should fail with a bad username");

  shutdown (c->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_authenticate (c, config.db, "test", "s3kr1+") == TRUE,
      "mongo_sync_cmd_authenticate() automatically reconnects");

  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_authenticate_net_secondary ();

  end_network_tests ();
}

void
test_mongo_sync_cmd_authenticate (void)
{
  mongo_sync_connection *c;

  c = test_make_fake_sync_conn (-1, FALSE);

  errno = 0;
  ok (mongo_sync_cmd_authenticate (NULL, "test", "test",
					"s3kr1+") == FALSE,
      "mongo_sync_cmd_authenticate() fails with a NULL connection");
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is set to ENOTCONN");

  errno = 0;
  ok (mongo_sync_cmd_authenticate (c, NULL, "test", "s3kr1+") == FALSE,
      "mongo_sync_cmd_authenticate() fails with a NULL db");
  cmp_ok (errno, "==", EINVAL,
	  "errno is set to EINVAL");

  errno = 0;
  ok (mongo_sync_cmd_authenticate (c, "test", NULL, "s3kr1+") == FALSE,
      "mongo_sync_cmd_authenticate() fails with a NULL user");
  cmp_ok (errno, "==", EINVAL,
	  "errno is set to EINVAL");

  errno = 0;
  ok (mongo_sync_cmd_authenticate (c, "test", "test", NULL) == FALSE,
      "mongo_sync_cmd_authenticate() fails with a NULL password");
  cmp_ok (errno, "==", EINVAL,
	  "errno is set to EINVAL");

  ok (mongo_sync_cmd_authenticate (c, "test", "test",
					"s3kr1+") == FALSE,
      "mongo_sync_cmd_authenticate() fails with a bogus FD");

  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_authenticate_net ();
}

RUN_TEST (17, mongo_sync_cmd_authenticate);
#else
void
test_mongo_sync_cmd_authenticate  (void)
{
  errno = 0;
  if (mongo_sync_cmd_authenticate (NULL, NULL, NULL, NULL) != FALSE)
    fail ("mongo_sync_authenticate() with NULLs should fail");
  else
    ok (errno == ENOTSUP,
	"mongo_sync_authenticate() should fail with ENOTSUP "
	"when authentication is not compiled in");
}

RUN_TEST (1, mongo_sync_cmd_authenticate);
#endif
