#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_get_set_auto_reconnect (void)
{
  mongo_sync_connection *c;

  c = test_make_fake_sync_conn (-1, FALSE);

  errno = 0;
  ok (mongo_sync_conn_get_auto_reconnect (NULL) == FALSE,
      "mongo_sync_conn_get_auto_reconnect() returns FALSE with a "
      "NULL connection");
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is now set to ENOTCONN");

  ok (mongo_sync_conn_get_auto_reconnect (c) == FALSE,
      "mongo_sync_get_auto_reconnect() works");
  cmp_ok (errno, "==", 0,
	  "errno is now cleared");

  errno = 0;
  mongo_sync_conn_set_auto_reconnect (NULL, TRUE);
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is set to ENOTCONN after "
	  "mongo_sync_conn_set_auto_reconnect(NULL)");

  ok (mongo_sync_conn_set_auto_reconnect (c, TRUE),
      "mongo_sync_auto_reconnect() works");
  ok (mongo_sync_conn_get_auto_reconnect (c) == TRUE,
      "mongo_sync_set_auto_reconnect() worked");

  mongo_sync_disconnect (c);
}

RUN_TEST (7, mongo_sync_get_set_auto_reconnect);
