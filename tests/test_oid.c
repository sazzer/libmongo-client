#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "mongo-utils.h"

void
test_oid (void)
{
  guint8 *oid, *oid2;

  TEST (mongo_util.oid);
  oid = mongo_util_oid_new (42);
  g_assert (oid[11] == 42);
  g_free (oid);
  PASS ();

  TEST(mongo_util.oid_inc);
  oid = mongo_util_oid_new (42);
  g_assert (oid[11] == 42);
  sleep (2);
  oid2 = mongo_util_oid_new (128);
  g_assert (oid2[11] == 128);
  g_assert_cmpint (memcmp (oid, oid2, 12), <, 0);
  g_free (oid);
  g_free (oid2);
  PASS ();
}

int
main (void)
{
  test_oid ();
  return 0;
}
