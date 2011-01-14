#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "mongo-utils.h"

void
test_oid (void)
{
  guint8 *oid;

  TEST (mongo_util.oid);
  oid = mongo_util_oid_new (42);
  g_free (oid);
  PASS ();
}

int
main (void)
{
  test_oid ();
  return 0;
}
