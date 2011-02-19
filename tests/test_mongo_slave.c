#include "test-network.h"
#include "test-generator.h"

#include "mongo.h"

#include <errno.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <sys/socket.h>

void
test_mongo_slave_setup (void)
{
  bson *doc;
  mongo_sync_connection *conn;
  mongo_packet *p;

  TEST (mongo_slave.setup);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  doc = test_bson_generate_flat ();

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
  g_assert_cmpint (cnt, ==, -1);

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
  g_assert ((conn = mongo_sync_reconnect (conn, FALSE)) != NULL);
  g_assert (o == conn);
  g_assert ((conn = mongo_sync_reconnect (conn, TRUE)) != NULL);
  g_assert (o != conn);
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
do_plan (int max)
{
  mongo_sync_connection *conn;

  if (!test_getenv_server ())
    SKIP_ALL ("TEST_SERVER variable not set");
  if (!test_getenv_secondary ())
    SKIP_ALL ("TEST_SECONDARY variable not set");

  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, FALSE);
  if (!conn)
    SKIP_ALL ("cannot connect to mongodb");

  PLAN (1, max);
  mongo_sync_disconnect (conn);
}

int
main (void)
{
  mongo_util_oid_init (0);
  do_plan (10);

  test_mongo_slave_setup ();
  test_mongo_slave_cmd_count ();
  test_mongo_slave_fail ();
  test_mongo_slave_cmd_is_master ();
  test_mongo_slave_reconnect ();
  test_mongo_slave_teardown ();

  test_env_free ();

  return 0;
}
