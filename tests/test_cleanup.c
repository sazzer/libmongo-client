#include "test.h"
#include <mongo.h>

int
main (void)
{
  mongo_sync_connection *conn;

  if (!test_env_setup ())
    return 0;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  mongo_sync_cmd_drop (conn, config.db, config.coll);

  test_env_free ();

  return 0;
}
