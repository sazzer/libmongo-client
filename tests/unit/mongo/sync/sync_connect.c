#include "test.h"
#include "mongo.h"

void
test_mongo_sync_connect (void)
{
  mongo_sync_connection *c;

  ok (mongo_sync_connect (NULL, 27017, FALSE) == NULL,
      "mongo_sync_connect() fails with a NULL host");

  begin_network_tests (1);

  ok ((c = mongo_sync_connect (config.primary_host,
			       config.primary_port, FALSE)) != NULL,
      "mongo_sync_connect() works");
  mongo_sync_disconnect (c);

  end_network_tests ();
}

RUN_TEST (2, mongo_sync_connect);
