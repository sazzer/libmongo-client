#include "test.h"
#include "mongo.h"

#include <sys/socket.h>
#include "libmongo-private.h"

void
test_mongo_sync_cmd_insert_n (void)
{
  mongo_sync_connection *c;
  bson *b1, *b2, *b3;
  const bson *docs[10];

  c = test_make_fake_sync_conn (-1, FALSE);
  b1 = test_bson_generate_full ();
  b2 = test_bson_generate_full ();
  b3 = bson_new ();

  docs[0] = b1;
  docs[1] = b2;
  docs[2] = b3;
  docs[3] = NULL;
  docs[4] = b1;

  ok (mongo_sync_cmd_insert_n (NULL, "test.ns", 3, docs) == FALSE,
      "mongo_sync_cmd_insert_n() fails with a NULL connection");
  ok (mongo_sync_cmd_insert_n (c, NULL, 3, docs) == FALSE,
      "mongo_sync_cmd_insert_n() fails with a NULL namespace");
  ok (mongo_sync_cmd_insert_n (c, "test.ns", 0, docs) == FALSE,
      "mongo_sync_cmd_insert_n() fails with no documents to insert");
  ok (mongo_sync_cmd_insert_n (c, "test.ns", 3, NULL) == FALSE,
      "mongo_sync_cmd_insert_n() fails with no documents to insert");
  ok (mongo_sync_cmd_insert_n (c, "test.ns", 3, docs) == FALSE,
      "mongo_sync_cmd_insert_n() fails when the array contains an "
      "unfinished document");
  bson_finish (b3);
  ok (mongo_sync_cmd_insert_n (c, "test.ns", 5, docs) == FALSE,
      "mongo_sync_cmd_insert_n() fails when the array contains a "
      "NULL document");
  ok (mongo_sync_cmd_insert_n (c, "test.ns", 3, docs) == FALSE,
      "mongo_sync_cmd_insert_n() fails with a bogus FD");

  mongo_sync_disconnect (c);
  bson_free (b1);
  bson_free (b2);
  bson_free (b3);

  begin_network_tests (4);

  b1 = bson_new ();
  bson_append_string (b2, "sync_cmd_insert_n", "works", -1);
  bson_finish (b1);

  b2 = bson_new ();
  bson_append_int32 (b2, "int32", 1984);
  bson_finish (b2);

  docs[0] = b1;
  docs[1] = b2;

  c = mongo_sync_connect (config.primary_host, config.primary_port,
			  TRUE);

  ok (mongo_sync_cmd_insert_n (c, config.ns, 2, docs) == TRUE,
      "mongo_sync_cmd_insert_n() works");

  shutdown (c->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_insert_n (c, config.ns, 2, docs) == TRUE,
      "mongo_sync_cmd_insert_n() automatically reconnects");

  mongo_sync_disconnect (c);

  /*
   * Tests involving a secondary
   */
  skip (!config.secondary_host, 2, "Secondary host not set up");

  c = mongo_sync_connect (config.secondary_host, config.secondary_port,
			  TRUE);
  ok (c && mongo_sync_cmd_is_master (c) == FALSE,
      "Connected to a secondary");

  ok (mongo_sync_cmd_insert_n (c, config.ns, 2, docs) == TRUE,
      "mongo_sync_cmd_insert_n() automatically reconnects to master");
  mongo_sync_disconnect (c);

  endskip;

  bson_free (b1);
  bson_free (b2);

  end_network_tests ();
}

RUN_TEST (11, mongo_sync_cmd_insert_n);
