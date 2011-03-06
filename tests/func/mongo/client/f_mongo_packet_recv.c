#include "test.h"
#include "mongo-client.h"
#include "mongo-wire.h"
#include "bson.h"

#include "libmongo-private.h"

#include <sys/socket.h>

void
test_f_mongo_packet_recv (void)
{
  mongo_connection *conn;
  mongo_packet *p;
  bson *b;

  b = bson_new ();
  bson_append_int32 (b, "getnonce", 1);
  bson_finish (b);

  p = mongo_wire_cmd_custom (42, config.db, 0, b);
  bson_free (b);

  conn = mongo_connect (config.primary_host, config.primary_port);
  mongo_packet_send (conn, p);
  mongo_wire_packet_free (p);

  ok ((p = mongo_packet_recv (conn)) != NULL,
      "mongo_packet_recv() works");
  mongo_wire_packet_free (p);

  shutdown (conn->fd, SHUT_RDWR);
  sleep (1);

  ok (mongo_packet_recv (conn) == NULL,
      "mongo_packet_recv() fails on a closed socket");

  mongo_disconnect (conn);
}

RUN_NET_TEST (2, f_mongo_packet_recv);
