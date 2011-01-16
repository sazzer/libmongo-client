#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "mongo-utils.h"

void
test_oid_init (void)
{
  guint8 *oid;

  TEST (mongo_util.uninitilized);
  g_assert ((oid = mongo_util_oid_new (42)) == NULL);
  PASS ();

  TEST (mongo_util.init);
  mongo_util_oid_init (0);
  g_assert ((oid = mongo_util_oid_new (42)));
  g_free (oid);
  PASS ();
}

void
test_oid (void)
{
  guint8 *oid, *oid2;

  TEST (mongo_util.oid);
  g_assert ((oid = mongo_util_oid_new (42)));
  g_assert (oid[11] == 42);
  g_free (oid);
  PASS ();

  TEST(mongo_util.oid_inc);
  g_assert ((oid = mongo_util_oid_new (42)));
  g_assert (oid[11] == 42);
  sleep (2);
  oid2 = mongo_util_oid_new (128);
  g_assert (oid2[11] == 128);
  g_assert_cmpint (memcmp (oid, oid2, 12), <, 0);
  g_free (oid);
  g_free (oid2);
  PASS ();
}

void
test_oid_ts (void)
{
  guint8 *oid;
  gint32 ts = 1295198025;
  gint32 ts_be = GINT32_TO_BE (ts);

  TEST (mongo_util.oid_with_timestamp);
  g_assert ((oid = mongo_util_oid_new_with_time (ts, 42)));
  g_assert (oid[11] == 42);
  memcpy (&ts, oid, sizeof (ts));
  g_assert_cmpint (ts, ==, ts_be);
  g_free (oid);
  PASS ();
}

int
main (void)
{
  test_oid_init ();
  test_oid ();
  test_oid_ts ();
  return 0;
}
