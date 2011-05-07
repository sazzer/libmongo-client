#include "test.h"
#include "mongo.h"

#include "libmongo-private.h"

#include <sys/socket.h>

void
test_mongo_sync_cmd_update (void)
{
  mongo_sync_connection *c;
  bson *sel, *upd;
  guint8 *oid;

  mongo_util_oid_init (0);

  sel = bson_new ();
  oid = mongo_util_oid_new (0);
  bson_append_oid (sel, "_id", oid);
  g_free (oid);
  bson_finish (sel);

  upd = test_bson_generate_full ();
  c = test_make_fake_sync_conn (-1, FALSE);

  ok (mongo_sync_cmd_update (NULL, "test.ns", 0, sel, upd) == FALSE,
      "mongo_sync_cmd_update() fails with a NULL connection");
  ok (mongo_sync_cmd_update (c, NULL, 0, sel, upd) == FALSE,
      "mongo_sync_cmd_update() fails with a NULL namespace");
  ok (mongo_sync_cmd_update (c, "test.ns", 0, NULL, upd) == FALSE,
      "mongo_sync_cmd_update() fails with a NULL selector");
  ok (mongo_sync_cmd_update (c, "test.ns", 0, sel, NULL) == FALSE,
      "mongo_sync_cmd_update() fails with a NULL update");

  ok (mongo_sync_cmd_update (c, "test.ns", 0, sel, upd) == FALSE,
      "mongo_sync_cmd_update() fails with a bogus FD");

  mongo_sync_disconnect (c);
  bson_free (sel);
  bson_free (upd);

  begin_network_tests (4);

  sel = bson_new ();
  oid = mongo_util_oid_new (1);
  bson_append_oid (sel, "_id", oid);
  g_free (oid);
  bson_finish (sel);

  upd = bson_new ();
  oid = mongo_util_oid_new (1);
  bson_append_oid (upd, "_id", oid);
  g_free (oid);
  bson_finish (upd);

  c = mongo_sync_connect (config.primary_host, config.primary_port,
			  FALSE);
  mongo_sync_conn_set_auto_reconnect (c, TRUE);

  ok (mongo_sync_cmd_update (c, config.ns,
			     MONGO_WIRE_FLAG_UPDATE_UPSERT, sel, upd) == TRUE,
      "mongo_sync_cmd_update() works");

  shutdown (c->super.fd, SHUT_RDWR);
  sleep (3);

  ok (mongo_sync_cmd_update (c, config.ns,
			     MONGO_WIRE_FLAG_UPDATE_UPSERT, sel, upd) == TRUE,
      "mongo_sync_cmd_update() automatically reconnects");

  mongo_sync_disconnect (c);

  /*
   * Tests involving a secondary
   */
  skip (!config.secondary_host, 2,
	"Secondary host not set up");

  c = mongo_sync_connect (config.secondary_host, config.secondary_port,
			  TRUE);
  mongo_sync_conn_set_auto_reconnect (c, TRUE);

  ok (mongo_sync_cmd_is_master (c) == FALSE,
      "Connected to a secondary");

  ok (mongo_sync_cmd_update (c, config.ns,
			     MONGO_WIRE_FLAG_UPDATE_UPSERT, sel, upd) == TRUE,
      "mongo_sync_cmd_update() automatically reconnects to master");
  mongo_sync_disconnect (c);
  endskip;

  bson_free (sel);
  bson_free (upd);
  end_network_tests ();
}

RUN_TEST (9, mongo_sync_cmd_update);
