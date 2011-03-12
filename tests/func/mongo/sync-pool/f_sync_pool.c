#include "test.h"
#include <mongo.h>

#include <errno.h>

#include "libmongo-private.h"

void
test_func_mongo_sync_pool (void)
{
  mongo_sync_pool *pool;
  mongo_sync_pool_connection *conn[11], *t;
  gint c = 0;
  gboolean ret = TRUE;
  bson *b;
  mongo_packet *p;

  /*
   * First, we test whether the basics work, like connecting, picking
   * & returning.
   */

  pool = mongo_sync_pool_new (config.primary_host,
			      config.primary_port,
			      10, 0);

  ok (pool != NULL,
      "mongo_sync_pool_new() works");

  while ((conn[c] = mongo_sync_pool_pick (pool, TRUE)) != NULL)
    c++;
  cmp_ok (c, "==", 10,
	  "Successfully connect to the master on 10 sockets");

  t = mongo_sync_pool_pick (pool, TRUE);
  ok (t == NULL && errno == EAGAIN,
      "Connected to the master only on 10 sockets");

  for (c = 0; c < 10; c++)
    ret = ret && mongo_sync_pool_return (pool, conn[c]);
  ok (ret == TRUE,
      "mongo_sync_pool_return() works");

  t = mongo_sync_pool_pick (pool, TRUE);
  ok (t != NULL,
      "mongo_sync_pool_pick() works after returning connections");
  mongo_sync_pool_return (pool, t);

  /*
   * Then we test whether we can perform commands on random
   * connections.
   */
  conn[0] = mongo_sync_pool_pick (pool, TRUE);
  conn[1] = mongo_sync_pool_pick (pool, TRUE);

  ok (conn[0] != conn[1],
      "Two picked connections are not the same");

  b = bson_build (BSON_TYPE_STRING, "test-name", __FILE__, -1,
		  BSON_TYPE_INT32, "i32", 1984,
		  BSON_TYPE_NONE);
  bson_finish (b);

  ok (mongo_sync_cmd_insert ((mongo_sync_connection *)conn[0],
			     config.ns, b, NULL) == TRUE,
      "mongo_sync_cmd_insert() works on a picked connection");

  p = mongo_sync_cmd_query ((mongo_sync_connection *)conn[1],
			    config.ns, 0, 0, 1, b, NULL);
  ok (p != NULL,
      "mongo_sync_cmd_query() works on a different picked connection");
  mongo_wire_packet_free (p);

  mongo_sync_pool_free (pool);
}

RUN_NET_TEST (8, func_mongo_sync_pool);
