#include "test.h"
#include <mongo.h>

#include <errno.h>

#include "libmongo-private.h"

void
test_func_mongo_sync_max_insert_size (void)
{
  mongo_sync_connection *conn;
  const bson *docs[10];
  bson *b1, *b2, *b3;

  b1 = test_bson_generate_full ();
  b2 = test_bson_generate_full ();
  b3 = bson_new ();
  bson_finish (b3);

  conn = mongo_sync_connect (config.primary_host, config.primary_port,
			     FALSE);

  /*
   * cmd_insert_n()
   */
  mongo_sync_conn_set_max_insert_size (conn, bson_size (b1) +
				       bson_size (b3) + 1);

  docs[0] = b1;
  docs[1] = b2;
  docs[2] = b3;

  ok (mongo_sync_cmd_insert_n (conn, config.ns, 3, docs) == TRUE,
      "mongo_sync_cmd_insert_n() works with a small max_insert_size");

  mongo_sync_conn_set_max_insert_size (conn, 1);
  errno = 0;
  ok (mongo_sync_cmd_insert_n (conn, config.ns, 3, docs) == FALSE,
      "mongo_sync_cmd_insert_n() should fail if any one document is too big");
  cmp_ok (errno, "==", EMSGSIZE,
	  "errno is set to EMSGSIZE");

  /*
   * cmd_insert()
   */
  mongo_sync_conn_set_max_insert_size (conn, bson_size (b1) +
				       bson_size (b3) + 1);
  ok (mongo_sync_cmd_insert (conn, config.ns, b1, b2, b3, NULL) == TRUE,
      "mongo_sync_cmd_insert() works with a small max_insert_size");

  mongo_sync_conn_set_max_insert_size (conn, 1);
  errno = 0;
  ok (mongo_sync_cmd_insert (conn, config.ns, b1, b2, b3, NULL) == FALSE,
      "mongo_sync_cmd_insert() should fail if any one document is too big");
  cmp_ok (errno, "==", EMSGSIZE,
	  "errno is set to EMSGSIZE");

  mongo_sync_disconnect (conn);
  bson_free (b1);
  bson_free (b2);
  bson_free (b3);
}

RUN_NET_TEST (6, func_mongo_sync_max_insert_size);
