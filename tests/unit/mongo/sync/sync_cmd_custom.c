#include "test.h"
#include "mongo.h"

#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_custom_net_secondary (void)
{
  mongo_sync_connection *conn;
  bson *cmd;
  mongo_packet *p;

  skip (!config.secondary_host, 1,
	"Secondary server not configured");

  conn = mongo_sync_connect (config.secondary_host, config.secondary_port,
			     TRUE);
  cmd = bson_build (BSON_TYPE_INT32, "getnonce", 1,
		    BSON_TYPE_NONE);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, config.db, cmd);
  ok (p != NULL,
      "mongo_sync_cmd_custom() works on the secondary too");
  mongo_wire_packet_free (p);

  bson_free (cmd);
  mongo_sync_disconnect (conn);

  endskip;
}

void
test_mongo_sync_cmd_custom_net (void)
{
  mongo_sync_connection *conn;
  bson *cmd;
  mongo_packet *p;

  begin_network_tests (3);

  conn = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  mongo_sync_cmd_is_master (conn);

  cmd = bson_build (BSON_TYPE_INT32, "getnonce", 1,
		    BSON_TYPE_NONE);
  bson_finish (cmd);

  p = mongo_sync_cmd_custom (conn, config.db, cmd);
  ok (p != NULL,
      "mongo_sync_cmd_custom() works");
  mongo_wire_packet_free (p);

  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (3);

  p = mongo_sync_cmd_custom (conn, config.db, cmd);
  ok (p != NULL,
      "mongo_sync_cmd_custom() automatically reconnects");
  mongo_wire_packet_free (p);

  bson_free (cmd);
  mongo_sync_disconnect (conn);

  test_mongo_sync_cmd_custom_net_secondary ();

  end_network_tests ();
}

void
test_mongo_sync_cmd_custom (void)
{
  mongo_sync_connection *c;
  bson *cmd;

  c = test_make_fake_sync_conn (-1, FALSE);
  cmd = bson_new ();
  bson_append_int32 (cmd, "getnonce", 1);
  bson_finish (cmd);

  ok (mongo_sync_cmd_custom (NULL, "test", cmd) == NULL,
      "mongo_sync_cmd_custom() fails with a NULL connection");
  ok (mongo_sync_cmd_custom (c, NULL, cmd) == NULL,
      "mongo_sync_cmd_custom() fails with a NULL namespace");

  ok (mongo_sync_cmd_custom (c, "test", cmd) == NULL,
      "mongo_sync_cmd_custom() fails with a bogus FD");
  mongo_sync_conn_set_slaveok (c, TRUE);
  ok (mongo_sync_cmd_custom (c, "test", cmd) == NULL,
      "mongo_sync_cmd_custom() fails with a bogus FD");

  bson_free (cmd);
  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_custom_net ();
}

RUN_TEST (7, mongo_sync_cmd_custom);
