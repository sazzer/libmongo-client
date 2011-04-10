#include "test.h"
#include "mongo.h"

#include "libmongo-private.h"

void
test_mongo_connection_get_requestid (void)
{
  mongo_connection c, *conn;
  mongo_packet *p;
  bson *b;
  gint reqid;

  c.request_id = 42;

  ok (mongo_connection_get_requestid (NULL) == -1,
      "mongo_connection_get_requestid() fails with a NULL connection");
  ok (mongo_connection_get_requestid (&c) == 42,
      "mongo_connection_get_requestid() works");

  begin_network_tests (2);

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

  end_network_tests ();
}

RUN_TEST (4, mongo_connection_get_requestid);
