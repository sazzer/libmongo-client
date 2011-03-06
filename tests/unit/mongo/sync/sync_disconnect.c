#include "test.h"
#include "mongo.h"

#include "libmongo-private.h"

void
test_mongo_sync_disconnect (void)
{
  mongo_sync_connection *conn;

  mongo_sync_disconnect (NULL);
  pass ("mongo_sync_disconnect(NULL) does not crash");

  conn = test_make_fake_sync_conn (-1, FALSE);
  conn->rs.hosts = g_list_append (conn->rs.hosts,
				  g_strdup ("invalid.example.com:-42"));

  mongo_sync_disconnect (conn);
  pass ("mongo_sync_disconnect() works");
}

RUN_TEST (2, mongo_sync_disconnect);
