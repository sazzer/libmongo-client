#include "test.h"
#include "mongo.h"

#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_drop_net_secondary (void)
{
  mongo_sync_connection *conn;
  bson *b;
  gboolean ret;

  skip (!config.secondary_host, 1,
	"Secondary server not configured");

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  b = bson_build (BSON_TYPE_BOOLEAN, "filler", TRUE,
		  BSON_TYPE_NONE);
  bson_finish (b);
  mongo_sync_cmd_insert (conn, config.ns, b, NULL);
  bson_free (b);
  mongo_sync_disconnect (conn);

  conn = mongo_sync_connect (config.secondary_host, config.secondary_port,
			     TRUE);
  mongo_sync_cmd_is_master (conn);

  ret = mongo_sync_cmd_drop (conn, config.db, config.coll);
  ok (ret && mongo_sync_cmd_is_master (conn),
      "mongo_sync_cmd_drop() can reconnect to master");
  mongo_sync_disconnect (conn);

  endskip;
}

void
test_mongo_sync_cmd_drop_net (void)
{
  mongo_sync_connection *conn;
  bson *b;

  begin_network_tests (3);

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  b = bson_build (BSON_TYPE_BOOLEAN, "filler", TRUE,
		  BSON_TYPE_NONE);
  bson_finish (b);
  mongo_sync_cmd_insert (conn, config.ns, b, NULL);

  ok (mongo_sync_cmd_drop (conn, config.db, config.coll) == TRUE,
      "mongo_sync_cmd_drop() works");

  mongo_sync_cmd_insert (conn, config.ns, b, NULL);

  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_drop (conn, config.db, config.coll) == TRUE,
      "mongo_sync_cmd_drop() automatically reconnects");

  bson_free (b);
  mongo_sync_disconnect (conn);

  test_mongo_sync_cmd_drop_net_secondary ();

  end_network_tests ();
}

void
test_mongo_sync_cmd_drop (void)
{
  mongo_sync_connection *c;

  c = test_make_fake_sync_conn (-1, FALSE);

  ok (mongo_sync_cmd_drop (NULL, "test", "db") == FALSE,
      "mongo_sync_cmd_drop() fails with a NULL connection");
  ok (mongo_sync_cmd_drop (c, NULL, "db") == FALSE,
      "mongo_sync_cmd_drop() fails with a NULL db");

  ok (mongo_sync_cmd_drop (c, "test", "db") == FALSE,
      "mongo_sync_cmd_drop() fails with a bogus FD");

  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_drop_net ();
}

RUN_TEST (6, mongo_sync_cmd_drop);
