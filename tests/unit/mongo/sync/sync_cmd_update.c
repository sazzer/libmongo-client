#include "test.h"
#include "mongo.h"

void
test_mongo_sync_cmd_update (void)
{
  mongo_sync_connection *c;
  bson *sel, *upd;

  test_env_setup ();

  sel = test_bson_generate_full ();
  upd = test_bson_generate_full ();
  c = test_make_fake_sync_conn (-1, FALSE);

  ok (mongo_sync_cmd_update (NULL, config.ns, 0, sel, upd) == FALSE,
      "mongo_sync_cmd_update() fails with a NULL connection");
  ok (mongo_sync_cmd_update (c, NULL, 0, sel, upd) == FALSE,
      "mongo_sync_cmd_update() fails with a NULL namespace");
  ok (mongo_sync_cmd_update (c, config.ns, 0, NULL, upd) == FALSE,
      "mongo_sync_cmd_update() fails with a NULL selector");
  ok (mongo_sync_cmd_update (c, config.ns, 0, sel, NULL) == FALSE,
      "mongo_sync_cmd_update() fails with a NULL update");

  ok (mongo_sync_cmd_update (c, config.ns, 0, sel, upd) == FALSE,
      "mongo_sync_cmd_update() fails with a bogus FD");

  mongo_sync_disconnect (c);
  bson_free (sel);
  bson_free (upd);

  test_env_free ();
}

RUN_TEST (5, mongo_sync_cmd_update);
