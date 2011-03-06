#include "test.h"
#include "mongo-client.h"

void
test_f_mongo_connect (void)
{
  mongo_connection *c;

  ok (mongo_connect ("invalid.example.com", 27017) == NULL,
      "Connecting to an invalid host fails");
  ok (mongo_connect ("example.com", 27017) == NULL,
      "Connecting to an unavailable host/port fails");

  ok ((c = mongo_connect (config.primary_host,
			  config.primary_port)) != NULL,
      "Connecting to the primary server works");
  mongo_disconnect (c);
}

RUN_NET_TEST (3, f_mongo_connect);
