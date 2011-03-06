#include "test.h"
#include "mongo-client.h"
#include "mongo-wire.h"
#include "bson.h"

void
test_f_mongo_connection_get_requestid (void)
{
  mongo_connection *conn;
  mongo_packet *p;
  bson *b;
  gint reqid;

  b = bson_new ();
  bson_append_int32 (b, "getnonce", 1);
  bson_finish (b);

  p = mongo_wire_cmd_custom (42, config.db, 0, b);
  bson_free (b);

  conn = mongo_connect (config.primary_host, config.primary_port);
  cmp_ok ((reqid = mongo_connection_get_requestid (conn)), "==", 0,
	  "Initial request id is 0");
  mongo_packet_send (conn, p);
  mongo_wire_packet_free (p);

  cmp_ok (reqid, "<", mongo_connection_get_requestid (conn),
	  "Old request ID is smaller than the new one");

  mongo_disconnect (conn);
}

RUN_NET_TEST (2, f_mongo_connection_get_requestid);
