#include "test.h"
#include "tap.h"
#include "mongo-client.h"

#include <errno.h>

void
test_mongo_connect (void)
{
  mongo_connection *c;

  ok (mongo_connect (NULL, 27010) == NULL,
      "mongo_connect() fails with a NULL host");
  ok (errno == EINVAL,
      "mongo_connect() should fail with EINVAL if host is NULL");

  begin_network_tests (3);

  ok (mongo_connect ("invalid.example.com", 27017) == NULL,
      "Connecting to an invalid host fails");
  ok (mongo_connect ("example.com", 27017) == NULL,
      "Connecting to an unavailable host/port fails");

  ok ((c = mongo_connect (config.primary_host,
			  config.primary_port)) != NULL,
      "Connecting to the primary server works");
  mongo_disconnect (c);

  end_network_tests ();
}

RUN_TEST (5, mongo_connect);
