#include "test.h"
#include "mongo.h"

#include <errno.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <sys/socket.h>

#define TEST(n)
#define PASS() pass()

#define TEST_SERVER_IP config.primary_host
#define TEST_SERVER_PORT config.primary_port
#define TEST_SERVER_DB config.db
#define TEST_SERVER_COLLECTION config.coll
#define TEST_SERVER_NS config.ns

#define TEST_SECONDARY_IP config.secondary_host
#define TEST_SECONDARY_PORT config.secondary_port

void
test_mongo_slave_setup (void)
{
  bson *doc;
  mongo_sync_connection *conn;
  mongo_packet *p;

  TEST (mongo_slave.setup);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  doc = test_bson_generate_full ();

  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc, NULL));
  bson_free (doc);

  /* Run fsync on the admin db, so that we sync to the replica set.
   * Otherwise we might end up in an inconsistent state when we reach
   * the later test cases, and we'll fail.
   */
  doc = bson_new ();
  bson_append_int32 (doc, "fsync", 1);
  bson_finish (doc);
  g_assert ((p = mongo_sync_cmd_custom (conn, "admin", doc)) != NULL);
  mongo_wire_packet_free (p);
  bson_free (doc);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_slave_teardown (void)
{
  mongo_sync_connection *conn;

  TEST (mongo_slave.teardown);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_drop (conn, TEST_SERVER_DB,
				 TEST_SERVER_COLLECTION));

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_slave_cmd_count (void)
{
  gdouble cnt;
  mongo_sync_connection *conn;

  TEST (mongo_slave.cmd_count);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  cnt = mongo_sync_cmd_count (conn, TEST_SERVER_DB, TEST_SERVER_COLLECTION,
			      NULL);

  g_assert_cmpint (cnt, >, 0);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_slave_fail (void)
{
  gdouble cnt;
  mongo_sync_connection *conn;

  TEST (mongo_slave.fail);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, FALSE);
  g_assert (conn);

  cnt = mongo_sync_cmd_count (conn, TEST_SERVER_DB, TEST_SERVER_COLLECTION,
			      NULL);
  if (!mongo_sync_cmd_is_master (conn))
    g_assert_cmpint (cnt, ==, -1);
  else
    g_assert_cmpint (cnt, ==, 1);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_slave_cmd_is_master (void)
{
  mongo_sync_connection *conn;

  TEST (mongo_slave.cmd.is_master.primary);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn));

  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.cmd.is_master.secondary);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);
  PASS ();

  TEST (mongo_slave.cmd.is_master.secondary.reconnect);
  conn = mongo_sync_reconnect (conn, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn));

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_slave_reconnect (void)
{
  mongo_sync_connection *conn, *o;
  gint i;

  TEST (mongo_slave.reconnect.primary);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  o = conn;
  g_assert ((conn = mongo_sync_reconnect (conn, FALSE)) != NULL);
  g_assert (o == conn);
  g_assert ((conn = mongo_sync_reconnect (conn, TRUE)) != NULL);
  g_assert (o == conn);

  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.reconnect.secondary);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, FALSE);
  g_assert (conn);

  o = conn;
  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);
  g_assert ((conn = mongo_sync_reconnect (conn, FALSE)) != NULL);
  g_assert (o == conn);
  g_assert ((conn = mongo_sync_reconnect (conn, TRUE)) != NULL);
  g_assert (o == conn);
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);

  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.reconnect.from_disconnect);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  for (i = 3; i < 128; i++)
    shutdown (i, SHUT_RDWR);

  g_assert (mongo_sync_cmd_ping (conn) == FALSE);
  conn = mongo_sync_reconnect (conn, TRUE);
  g_assert (conn);
  g_assert (mongo_sync_cmd_ping (conn));

  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_slave_auto_reconnect (void)
{
  mongo_sync_connection *conn;
  bson *b, *u;
  mongo_packet *p;

  TEST (mongo_slave.auto_reconnect.insert);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  b = bson_build (BSON_TYPE_INT32, "auto_sync", 1234,
		  BSON_TYPE_NONE);
  bson_finish (b);

  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, b, NULL));
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);

  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.auto_reconnect.update);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  u = bson_build (BSON_TYPE_INT32, "auto_sync", 4321,
		  BSON_TYPE_NONE);
  bson_finish (u);

  g_assert (mongo_sync_cmd_update (conn, TEST_SERVER_NS, 0, b, u));
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);
  bson_free (b);

  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.auto_reconnect.query);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);
  g_assert ((p = mongo_sync_cmd_query (conn, TEST_SERVER_NS, 0, 0, 1,
				       u, NULL)) != NULL);
  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  mongo_wire_packet_free (p);
  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.auto_reconnect.query.force_master);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);
  g_assert ((p = mongo_sync_cmd_query (conn, TEST_SERVER_NS, 0, 0, 1,
				       u, NULL)) != NULL);
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);

  mongo_wire_packet_free (p);
  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.auto_reconnect.delete);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);
  g_assert (mongo_sync_cmd_delete (conn, TEST_SERVER_NS, 0, u));
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);

  bson_free (u);
  mongo_sync_disconnect (conn);
  PASS ();
}

void
test_mongo_slave_auto_reconnect_from_dc (void)
{
  mongo_sync_connection *conn;
  bson *b, *u;
  mongo_packet *p;
  gint i;

  TEST (mongo_slave.auto_reconnect.from_dc.insert);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  b = bson_build (BSON_TYPE_INT32, "auto_sync", 1234,
		  BSON_TYPE_NONE);
  bson_finish (b);

  for (i = 3; i < 128; i++)
    shutdown (i, SHUT_RDWR);

  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, b, NULL));
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);

  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.auto_reconnect.from_dc.update);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  u = bson_build (BSON_TYPE_INT32, "auto_sync", 4321,
		  BSON_TYPE_NONE);
  bson_finish (u);

  for (i = 3; i < 128; i++)
    shutdown (i, SHUT_RDWR);

  g_assert (mongo_sync_cmd_update (conn, TEST_SERVER_NS, 0, b, u));
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);
  bson_free (b);

  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.auto_reconnect.from_dc.query);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  for (i = 3; i < 128; i++)
    shutdown (i, SHUT_RDWR);

  g_assert ((p = mongo_sync_cmd_query (conn, TEST_SERVER_NS, 0, 0, 1,
				       u, NULL)) != NULL);

  mongo_wire_packet_free (p);
  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.auto_reconnect.from_dc.query.force_master);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, FALSE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  for (i = 3; i < 128; i++)
    shutdown (i, SHUT_RDWR);

  g_assert ((p = mongo_sync_cmd_query (conn, TEST_SERVER_NS, 0, 0, 1,
				       u, NULL)) != NULL);
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);

  mongo_wire_packet_free (p);
  mongo_sync_disconnect (conn);
  PASS ();

  TEST (mongo_slave.auto_reconnect.from_dc.delete);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  g_assert (mongo_sync_cmd_is_master (conn) == FALSE);

  for (i = 3; i < 128; i++)
    shutdown (i, SHUT_RDWR);

  g_assert (mongo_sync_cmd_delete (conn, TEST_SERVER_NS, 0, u));
  g_assert (mongo_sync_cmd_is_master (conn) == TRUE);

  bson_free (u);
  mongo_sync_disconnect (conn);
  PASS ();
}

static void
ignore_sigpipe (void)
{
  struct sigaction sa;

  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGPIPE, &sa, NULL);
}

void
test_f_mongo_sync_secondary_monolithic_mess (void)
{
  mongo_util_oid_init (0);
  ignore_sigpipe ();

  test_mongo_slave_setup ();
  test_mongo_slave_cmd_count ();
  test_mongo_slave_fail ();
  test_mongo_slave_cmd_is_master ();
  test_mongo_slave_reconnect ();

  test_mongo_slave_auto_reconnect ();
  test_mongo_slave_auto_reconnect_from_dc ();

  test_mongo_slave_teardown ();
}

RUN_NET_TEST (20, f_mongo_sync_secondary_monolithic_mess);
