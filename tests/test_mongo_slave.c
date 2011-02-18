#include "test.h"
#include "test-generator.h"

#include "mongo.h"

#include <errno.h>
#include <string.h>
#include <glib.h>

void
test_mongo_slave_setup (void)
{
  bson *doc;
  mongo_sync_connection *conn;

  TEST (mongo_slave.setup);
  conn = mongo_sync_connect (TEST_SERVER_IP, TEST_SERVER_PORT, FALSE);
  g_assert (conn);

  doc = test_bson_generate_flat ();

  g_assert (mongo_sync_cmd_insert (conn, TEST_SERVER_NS, doc));
  bson_free (doc);

  /* Run fsync on the admin db, so that we sync to the replica set.
   * Otherwise we might end up in an inconsistent state when we reach
   * the later test cases, and we'll fail.
   */
  doc = bson_new ();
  bson_append_int32 (doc, "fsync", 1);
  bson_finish (doc);
  g_assert (mongo_sync_cmd_custom (conn, "admin", doc));
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

  TEST (mongo_sync.cmd_count);
  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, TRUE);
  g_assert (conn);

  cnt = mongo_sync_cmd_count (conn, TEST_SERVER_DB, TEST_SERVER_COLLECTION,
			      NULL);
  
  g_assert_cmpint (cnt, >, 0);

  mongo_sync_disconnect (conn);
  PASS ();
}

void
do_plan (int max)
{
  mongo_sync_connection *conn;

  conn = mongo_sync_connect (TEST_SECONDARY_IP, TEST_SECONDARY_PORT, FALSE);
  if (!conn)
    SKIP_ALL ("cannot connect to mongodb; host="
	      TEST_SECONDARY_IP);

  PLAN (1, max);
  mongo_sync_disconnect (conn);
}

int
main (void)
{
  mongo_util_oid_init (0);
  do_plan (3);

  test_mongo_slave_setup ();
  test_mongo_slave_cmd_count ();
  test_mongo_slave_teardown ();

  return 0;
}
