#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_cmd_is_master_net_secondary (void)
{
  mongo_sync_connection *conn;

  skip (!config.secondary_host, 1,
	"Secondary server not configured");

  errno = 0;
  conn = mongo_sync_connect (config.secondary_host, config.secondary_port,
			     TRUE);
  ok (mongo_sync_cmd_is_master (conn) == FALSE && errno == 0,
      "mongo_sync_cmd_is_master() works correctly on a secondary");
  mongo_sync_disconnect (conn);

  endskip;
}

void
test_mongo_sync_cmd_is_master_net (void)
{
  mongo_sync_connection *conn;

  begin_network_tests (2);

  conn = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  ok (mongo_sync_cmd_is_master (conn) == TRUE,
      "mongo_sync_cmd_is_master() works");
  mongo_sync_disconnect (conn);

  test_mongo_sync_cmd_is_master_net_secondary ();

  end_network_tests ();
}

void
test_mongo_sync_cmd_is_master (void)
{
  mongo_sync_connection *c;

  c = test_make_fake_sync_conn (-1, FALSE);

  errno = 0;
  ok (mongo_sync_cmd_is_master (NULL) == FALSE,
      "mongo_sync_cmd_is_master fails with a NULL connection");
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is set to ENOTCONN");

  errno = 0;
  ok (mongo_sync_cmd_is_master (c) == FALSE,
      "mongo_sync_cmd_is_master() works");
  cmp_ok (errno, "!=", 0,
	  "errno is not 0");

  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_is_master_net ();
}

RUN_TEST (6, mongo_sync_cmd_is_master);
