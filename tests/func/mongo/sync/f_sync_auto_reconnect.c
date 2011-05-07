#include "test.h"
#include <mongo.h>

#include <errno.h>
#include <sys/socket.h>

#include "libmongo-private.h"

void
test_func_mongo_sync_auto_reconnect (void)
{
  mongo_sync_connection *conn;
  bson *b;
  mongo_packet *p;

  b = bson_new ();
  bson_append_int32 (b, "f_sync_auto_reconnect", 1);
  bson_finish (b);

  conn = mongo_sync_connect (config.primary_host, config.primary_port,
			     TRUE);
  ok (mongo_sync_cmd_insert (conn, config.ns, b, NULL) == TRUE);

  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (1);

  ok (mongo_sync_cmd_insert (conn, config.ns, b, NULL) == FALSE,
      "Inserting fails with auto-reconnect turned off, and a broken "
      "connection");

  mongo_sync_conn_set_auto_reconnect (conn, TRUE);

  ok (mongo_sync_cmd_insert (conn, config.ns, b, NULL) == TRUE,
      "Inserting works with auto-reconnect turned on, and a broken "
      "connection");

  mongo_sync_conn_set_auto_reconnect (conn, FALSE);

  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (1);

  ok (mongo_sync_cmd_insert (conn, config.ns, b, NULL) == FALSE,
      "Turning off auto-reconnect works");

  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (1);

  p = mongo_sync_cmd_query (conn, config.ns, 0, 0, 1, b, NULL);
  ok (p == NULL,
      "Query fails with auto-reconnect turned off");

  mongo_sync_conn_set_auto_reconnect (conn, TRUE);
  p = mongo_sync_cmd_query (conn, config.ns, 0, 0, 1, b, NULL);
  ok (p != NULL,
      "Query does reconnect with auto-reconnect turned on");
  mongo_wire_packet_free (p);

  mongo_sync_disconnect (conn);
}

RUN_NET_TEST (6, func_mongo_sync_auto_reconnect);
