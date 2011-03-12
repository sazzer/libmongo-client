#include "test.h"
#include "mongo.h"

#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_kill_cursors_net_secondary (void)
{
  mongo_packet *p;
  mongo_sync_connection *conn;
  bson *b;

  mongo_reply_packet_header rh;
  gint64 cid;

  skip (!config.secondary_host, 1,
	"Secondary server not configured");

  conn = mongo_sync_connect (config.secondary_host, config.secondary_port,
			     TRUE);
  b = bson_new ();
  bson_append_string (b, "test-name", __FILE__, -1);
  bson_finish (b);

  p = mongo_sync_cmd_query (conn, config.ns,
			    MONGO_WIRE_FLAG_QUERY_NO_CURSOR_TIMEOUT,
			    0, 2, b, NULL);
  bson_free (b);
  mongo_wire_reply_packet_get_header (p, &rh);
  cid = rh.cursor_id;
  mongo_wire_packet_free (p);

  ok (mongo_sync_cmd_kill_cursors (conn, 1, cid) == TRUE,
      "mongo_sync_cmd_kill_cursors() works on secondary too");

  mongo_sync_disconnect (conn);

  endskip;
}

void
test_mongo_sync_cmd_kill_cursors_net (void)
{
  mongo_packet *p;
  mongo_sync_connection *conn;
  bson *b;
  gint i;
  mongo_reply_packet_header rh;
  gint64 cid;

  begin_network_tests (3);

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

  p = mongo_sync_cmd_query (conn, config.ns,
			    MONGO_WIRE_FLAG_QUERY_NO_CURSOR_TIMEOUT,
			    0, 2, b, NULL);
  mongo_wire_reply_packet_get_header (p, &rh);
  cid = rh.cursor_id;
  mongo_wire_packet_free (p);

  ok (mongo_sync_cmd_kill_cursors (conn, 1, cid) == TRUE,
      "mongo_sync_kill_cursors() works");

  p = mongo_sync_cmd_query (conn, config.ns,
			    MONGO_WIRE_FLAG_QUERY_NO_CURSOR_TIMEOUT,
			    0, 2, b, NULL);
  bson_free (b);
  mongo_wire_reply_packet_get_header (p, &rh);
  cid = rh.cursor_id;
  mongo_wire_packet_free (p);
  shutdown (conn->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_kill_cursors (conn, 1, cid) == TRUE,
      "mongo_sync_cmd_kill_cursors() automatically reconnects");

  mongo_sync_disconnect (conn);

  test_mongo_sync_cmd_kill_cursors_net_secondary ();

  end_network_tests ();
}

void
test_mongo_sync_cmd_kill_cursors (void)
{
  mongo_sync_connection *c;

  c = test_make_fake_sync_conn (-1, FALSE);

  ok (mongo_sync_cmd_kill_cursors (NULL, 1, (gint64)1234) == FALSE,
      "mongo_sync_cmd_kill_cursors() fails with a NULL connection");
  ok (mongo_sync_cmd_kill_cursors (c, 0, (gint64)1234) == FALSE,
      "mongo_sync_cmd_kill_cursors() fails with a negative number of cursors");

  ok (mongo_sync_cmd_kill_cursors (c, 1, (gint64)1234) == FALSE,
      "mongo_sync_cmd_kill_cursors() fails with a bogus FD");

  mongo_sync_disconnect (c);

  test_mongo_sync_cmd_kill_cursors_net ();
}

RUN_TEST (6, mongo_sync_cmd_kill_cursors);
