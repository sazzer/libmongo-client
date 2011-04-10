#include "test.h"
#include "mongo.h"

#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_count_net_secondary (void)
{
  mongo_sync_connection *conn;
  bson *b;
  gdouble d;

  skip (!config.secondary_host, 2,
	"Secondary server not configured");

  conn = mongo_sync_connect (config.secondary_host, config.secondary_port,
			     TRUE);
  mongo_sync_cmd_is_master (conn);

  b = bson_new ();
  bson_append_string (b, "test-name", __FILE__, -1);
  bson_finish (b);

  d = mongo_sync_cmd_count (conn, config.db, config.coll, b);
  ok (d > 0,
      "mongo_sync_cmd_count() works on the secondary too");

  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (3);

  d = mongo_sync_cmd_count (conn, config.db, config.coll, b);
  ok (d > 0,
      "mongo_sync_cmd_count() automatically reconnects");

  bson_free (b);
  mongo_sync_disconnect (conn);

  endskip;
}

void
test_mongo_sync_cmd_count_net (void)
{
  mongo_sync_connection *conn;
  bson *b;
  gdouble d;
  gint i;

  begin_network_tests (4);

  conn = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  b = bson_new ();
  for (i = 0; i < 40; i++)
    {
      bson_reset (b);
      bson_append_string (b, "test-name", __FILE__, -1);
      bson_append_int32 (b, "seq", i);
      bson_finish (b);

      mongo_sync_cmd_insert (conn, config.ns, b, NULL);
    }
  bson_free (b);

  b = bson_new ();
  bson_append_string (b, "test-name", __FILE__, -1);
  bson_finish (b);

  d = mongo_sync_cmd_count (conn, config.db, config.coll, b);
  ok (d > 0,
      "mongo_sync_cmd_count() works");

  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (3);

  d = mongo_sync_cmd_count (conn, config.db, config.coll, b);
  ok (d > 0,
      "mongo_sync_cmd_count() automatically reconnects");

  bson_free (b);
  mongo_sync_disconnect (conn);

  test_mongo_sync_cmd_count_net_secondary ();

  end_network_tests ();
}

void
test_mongo_sync_cmd_count (void)
{
  mongo_sync_connection *c;
  bson *b;

  c = test_make_fake_sync_conn (-1, FALSE);
  b = test_bson_generate_full ();

  ok (mongo_sync_cmd_count (NULL, "test", "db", b) == -1,
      "mongo_sync_cmd_count() fails with a NULL connection");
  ok (mongo_sync_cmd_count (c, NULL, "db", b) == -1,
      "mongo_sync_cmd_count() fails with a NULL db");
  ok (mongo_sync_cmd_count (c, "test", NULL, b) == -1,
      "mongo_sync_cmd_count() fails with a NULL collection");

  ok (mongo_sync_cmd_count (c, "test", "db", b) == -1,
      "mongo_sync_cmd_count() fails with a bogus FD");
  mongo_sync_conn_set_slaveok (c, TRUE);
  ok (mongo_sync_cmd_count (c, "test", "db", b) == -1,
      "mongo_sync_cmd_count() fails with a bogus FD");

  bson_free (b);
  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_count_net ();
}

RUN_TEST (9, mongo_sync_cmd_count);
