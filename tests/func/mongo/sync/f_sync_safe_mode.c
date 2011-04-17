#include "test.h"
#include <mongo.h>

#include <errno.h>

#include "libmongo-private.h"

void
test_func_mongo_sync_safe_mode (void)
{
  mongo_sync_connection *conn;
  const bson *docs[10];
  bson *b1, *b2, *b3, *b4;

  mongo_util_oid_init (0);

  b1 = bson_new ();
  bson_append_string (b1, "func_mongo_sync_safe_mode", "works", -1);
  bson_finish (b1);

  b2 = bson_new ();
  bson_append_int32 (b2, "int32", 1984);
  bson_finish (b2);

  b3 = test_bson_generate_full ();
  b4 = test_bson_generate_full ();

  docs[0] = b1;
  docs[1] = b2;
  docs[2] = b3;
  docs[3] = b4;

  conn = mongo_sync_connect (config.primary_host, config.primary_port,
			     FALSE);

  /* Test inserts */
  mongo_sync_conn_set_safe_mode (conn, FALSE);
  ok (mongo_sync_cmd_insert_n (conn, config.ns, 4, docs) == TRUE,
      "mongo_sync_cmd_insert_n() should not fail with safe mode off");

  mongo_sync_conn_set_safe_mode (conn, TRUE);
  ok (mongo_sync_cmd_insert_n (conn, config.ns, 4, docs) == FALSE,
      "mongo_sync_cmd_insert_n() should fail with safe mode on");

  mongo_sync_disconnect (conn);
  bson_free (b1);
  bson_free (b2);
  bson_free (b3);
  bson_free (b4);
}

RUN_NET_TEST (2, func_mongo_sync_safe_mode);
