#include "test.h"
#include "mongo.h"

#include <errno.h>
#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_ping_net_secondary (void)
{
  mongo_sync_connection *c;

  skip (!config.secondary_host, 2,
	"Secondary server not configured");

  c = mongo_sync_connect (config.secondary_host, config.secondary_port, TRUE);

  ok (mongo_sync_cmd_ping (c) == TRUE,
      "mongo_sync_cmd_ping() works");

  shutdown (c->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_ping (c) == FALSE,
      "mongo_sync_cmd_ping() returns FALSE when not connected");

  mongo_sync_disconnect (c);

  endskip;
}

void
test_mongo_sync_cmd_ping_net (void)
{
  mongo_sync_connection *c;

  begin_network_tests (4);

  c = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);

  ok (mongo_sync_cmd_ping (c) == TRUE,
      "mongo_sync_cmd_ping() works");

  shutdown (c->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_ping (c) == FALSE,
      "mongo_sync_cmd_ping() returns FALSE when not connected");

  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_ping_net_secondary ();

  end_network_tests ();
}

void
test_mongo_sync_cmd_ping (void)
{
  mongo_sync_connection *c;

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

  test_mongo_sync_cmd_ping_net ();
}

RUN_TEST (8, mongo_sync_cmd_ping);
