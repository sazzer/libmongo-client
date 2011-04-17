#include "test.h"
#include <mongo.h>

#include "libmongo-private.h"

void
test_func_mongo_sync_conn_seed_add (void)
{
  mongo_sync_connection *conn;
  GList *l;

  conn = mongo_sync_connect (config.primary_host, config.primary_port,
			     FALSE);
  close (conn->super.fd);
  l = conn->rs.hosts;
  while (l)
    {
      g_free (l->data);
      l = g_list_delete_link (l, l);
    }
  conn->rs.hosts = NULL;

  conn = mongo_sync_reconnect (conn, TRUE);
  ok (conn == NULL,
      "mongo_sync_reconnect() fails without seeds or discovery");

  conn = mongo_sync_connect (config.primary_host, config.primary_port,
			     FALSE);
  close (conn->super.fd);
  l = conn->rs.hosts;
  while (l)
    {
      g_free (l->data);
      l = g_list_delete_link (l, l);
    }
  conn->rs.hosts = NULL;

  ok (mongo_sync_conn_seed_add (conn, config.primary_host,
				config.primary_port),
      "mongo_sync_conn_seed_add() works");

  conn = mongo_sync_reconnect (conn, TRUE);
  ok (conn != NULL,
      "mongo_sync_reconnect() works when properly seeded");

  mongo_sync_disconnect (conn);
}

RUN_NET_TEST (3, func_mongo_sync_conn_seed_add);
