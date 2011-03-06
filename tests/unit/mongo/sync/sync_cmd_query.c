#include "test.h"
#include "mongo.h"

#include <errno.h>
#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_query (void)
{
  mongo_packet *p;
  mongo_sync_connection *c;
  bson *q, *s;

  c = test_make_fake_sync_conn (-1, FALSE);
  q = test_bson_generate_full ();
  s = test_bson_generate_full ();

  ok (mongo_sync_cmd_query (NULL, "test.ns", 0, 0, 1, q, s) == NULL,
      "mongo_sync_cmd_query() fails with a NULL connection");
  ok (mongo_sync_cmd_query (c, NULL, 0, 0, 1, q, s) == NULL,
      "mongo_sync_cmd_query() fails with a NULL namespace");
  ok (mongo_sync_cmd_query (c, "test.ns", 0, 0, 1, NULL, s) == NULL,
      "mongo_sync_cmd_query() fails with a NULL query");

  ok (mongo_sync_cmd_query (c, "test.ns", 0, 0, 1, q, s) == NULL,
      "mongo_sync_cmd_query() fails with a bogus FD");
  mongo_sync_conn_set_slaveok (c, TRUE);
  ok (mongo_sync_cmd_query (c, "test.ns", 0, 0, 1, q, s) == NULL,
      "mongo_sync_cmd_query() fails with a bogus FD");

  mongo_sync_disconnect (c);

  bson_free (q);
  bson_free (s);

  begin_network_tests (7);

  q = bson_new ();
  bson_append_boolean (q, "sync_cmd_query_test", TRUE);
  bson_finish (q);

  s = bson_new ();
  bson_append_boolean (s, "sync_cmd_query_test", FALSE);
  bson_finish (s);

  c = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  mongo_sync_cmd_insert (c, config.ns, q, NULL);

  p = mongo_sync_cmd_query (c, config.ns, 0, 0, 1, q, NULL);
  ok (p != NULL,
      "mongo_sync_cmd_query() works");
  mongo_wire_packet_free (p);

  errno = 0;
  p = mongo_sync_cmd_query (c, config.ns, 0, 0, 1, s, NULL);
  ok (p == NULL && errno == ENOENT,
      "mongo_sync_cmd_query() sets errno to ENOENT when there's "
      "nothing to return");
  mongo_wire_packet_free (p);

  shutdown (c->super.fd, SHUT_RDWR);
  sleep (3);

  p = mongo_sync_cmd_query (c, config.ns, 0, 0, 1, q, NULL);
  ok (p != NULL,
      "mongo_sync_cmd_query() automatically reconnects");
  mongo_wire_packet_free (p);

  mongo_sync_disconnect (c);

  /*
   * Test request/response pairing, by sending a crafted query first,
   * and another, without reading the response for the first before
   * that.
   */
  c = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  p = mongo_wire_cmd_query (12345, config.ns, MONGO_WIRE_FLAG_QUERY_SLAVE_OK,
			    0, 1, s, NULL);
  mongo_packet_send ((mongo_connection *)c, p);
  mongo_wire_packet_free (p);

  errno = 0;
  p = mongo_sync_cmd_query (c, config.ns, 0, 0, 1, s, NULL);
  ok (p == NULL && errno == EPROTO,
      "mongo_sync_cmd_query() fails if the reply is not a response to "
      "the current query");
  mongo_wire_packet_free (p);

  mongo_sync_disconnect (c);

  /*
   * Tests involving a secondary
   */
  skip (!config.secondary_host, 3, "Secondary host not set up");

  c = mongo_sync_connect (config.secondary_host, config.secondary_port, TRUE);
  ok (c && mongo_sync_cmd_is_master (c) == FALSE,
      "Connected to a secondary");
  p = mongo_sync_cmd_query (c, config.ns, 0, 0, 1, s, NULL);
  ok (p != NULL,
      "mongo_sync_cmd_query() works on secondary");
  mongo_wire_packet_free (p);

  mongo_sync_conn_set_slaveok (c, FALSE);

  p = mongo_sync_cmd_query (c, config.ns, 0, 0, 1, s, NULL);
  ok (p != NULL && mongo_sync_cmd_is_master (c) == TRUE,
      "mongo_sync_cmd_query() can resync to master");
  mongo_wire_packet_free (p);

  mongo_sync_disconnect (c);

  endskip;

  bson_free (q);
  bson_free (s);

  end_network_tests ();
}

RUN_TEST (12, mongo_sync_cmd_query);
