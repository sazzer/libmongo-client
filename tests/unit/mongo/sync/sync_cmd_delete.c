#include "test.h"
#include "mongo.h"

#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_delete_net_secondary (void)
{
  mongo_sync_connection *conn;
  bson *b;

  skip (!config.secondary_host, 2,
	"Secondary server not configured");

  conn = mongo_sync_connect (config.primary_host, config.primary_port,
			     TRUE);

  b = bson_new ();
  bson_append_string (b, "unit-test", __FILE__, -1);
  bson_append_boolean (b, "delete-me", TRUE);
  bson_finish (b);
  mongo_sync_cmd_insert (conn, config.ns, b, NULL);

  mongo_sync_disconnect (conn);

  conn = mongo_sync_connect (config.secondary_host, config.secondary_port,
			     TRUE);
  ok (mongo_sync_cmd_delete (conn, config.ns, 0, b) == TRUE,
      "mongo_sync_cmd_delete() can reconnect to master");
  mongo_sync_disconnect (conn);

  conn = mongo_sync_connect (config.primary_host, config.primary_port,
			     TRUE);
  mongo_sync_cmd_insert (conn, config.ns, b, NULL);
  mongo_sync_disconnect (conn);

  conn = mongo_sync_connect (config.secondary_host, config.secondary_port,
			     TRUE);
  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_delete (conn, config.ns, 0, b) == FALSE,
      "mongo_sync_cmd_delete() fails if it can't reconnect to master");

  mongo_sync_disconnect (conn);
  bson_free (b);

  endskip;
}

void
test_mongo_sync_cmd_delete_net (void)
{
  mongo_sync_connection *conn;
  bson *b;

  begin_network_tests (4);

  conn = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);

  b = bson_new ();
  bson_append_string (b, "unit-test", __FILE__, -1);
  bson_append_boolean (b, "delete-me", TRUE);
  bson_finish (b);
  mongo_sync_cmd_insert (conn, config.ns, b, NULL);

  ok (mongo_sync_cmd_delete (conn, config.ns, 0, b) == TRUE,
      "mongo_sync_cmd_delete() works");

  mongo_sync_cmd_insert (conn, config.ns, b, NULL);

  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_delete (conn, config.ns, 0, b) == TRUE,
      "mongo_sync_cmd_delete() automatically reconnects");

  mongo_sync_disconnect (conn);
  bson_free (b);

  test_mongo_sync_cmd_delete_net_secondary ();

  end_network_tests ();
}

void
test_mongo_sync_cmd_delete (void)
{
  mongo_sync_connection *c;
  bson *b;

  c = test_make_fake_sync_conn (-1, FALSE);
  b = test_bson_generate_full ();

  ok (mongo_sync_cmd_delete (NULL, "test.ns", 0, b) == FALSE,
      "mongo_sync_cmd_delete() fails with a NULL connection");
  ok (mongo_sync_cmd_delete (c, NULL, 0, b) == FALSE,
      "mongo_sync_cmd_delete() fails with a NULL namespace");
  ok (mongo_sync_cmd_delete (c, "test.ns", 0, NULL) == FALSE,
      "mongo_sync_cmd_delete() fails with a NULL selector");

  ok (mongo_sync_cmd_delete (c, "test.ns", 0, b) == FALSE,
      "mongo_sync_cmd_delete() fails with a bogus FD");

  bson_free (b);
  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_delete_net ();
}

RUN_TEST (8, mongo_sync_cmd_delete);
