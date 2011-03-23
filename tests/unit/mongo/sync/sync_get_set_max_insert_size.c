#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_get_set_max_insert_size (void)
{
  mongo_sync_connection *c;

  c = test_make_fake_sync_conn (-1, FALSE);

  errno = 0;
  ok (mongo_sync_conn_get_max_insert_size (NULL) == -1,
      "mongo_sync_conn_get_max_insert_size() returns -1 with "
      "a NULL connection");

  cmp_ok (mongo_sync_conn_get_max_insert_size (c), "==",
	  MONGO_SYNC_DEFAULT_MAX_INSERT_SIZE,
      "mongo_sync_get_max_insert_size() works");

  errno = 0;
  mongo_sync_conn_set_max_insert_size (NULL, 1024);
  cmp_ok (errno, "==", ENOTCONN,
	  "errno is set to ENOTCONN after "
	  "mongo_sync_conn_set_max_insert_size(NULL)");

  mongo_sync_conn_set_max_insert_size (c, 1024);
  cmp_ok (errno, "==", 0,
	  "errno is cleared");
  ok (mongo_sync_conn_get_max_insert_size (c) == 1024,
      "mongo_sync_set_max_insert_size() worked");

  mongo_sync_conn_set_max_insert_size (c, -1);
  cmp_ok (errno, "==", ERANGE,
	  "errno is set to ERANGE");
  ok (mongo_sync_conn_get_max_insert_size (c) == 1024,
      "mongo_sync_set_max_insert_size() with a negative value should "
      "not work");

  mongo_sync_disconnect (c);
}

RUN_TEST (7, mongo_sync_get_set_max_insert_size);
