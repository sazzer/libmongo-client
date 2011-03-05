#include "tap.h"
#include "test.h"
#include "mongo-utils.h"

#include <string.h>
#include <unistd.h>

void
test_mongo_utils_oid_new (void)
{
  guint8 *oid1, *oid2, *oid3;

  ok (mongo_util_oid_new (0) == NULL,
      "mongo_util_oid_new() should fail before mongo_util_oid_init()");

  mongo_util_oid_init (0);
  ok ((oid1 = mongo_util_oid_new (1)) != NULL,
      "mongo_util_oid_new() works");
  cmp_ok (oid1[11], "==", 1,
	  "mongo_util_oid_new() returns an OID with the currect seq ID");

  oid2 = mongo_util_oid_new (2);
  oid3 = mongo_util_oid_new (2);

  ok (memcmp (oid2, oid1, 12) > 0,
      "OIDs with higher sequence ID sort higher");
  ok (memcmp (oid2, oid3, 12) == 0,
      "OIDs with the same sequence ID are equal (within a second)");
  g_free (oid2);
  g_free (oid3);

  sleep (2);
  oid2 = mongo_util_oid_new (0);

  ok (memcmp (oid2, oid1, 12) > 0,
      "OIDs with the same sequence ID, a few seconds later sort higher");
  g_free (oid2);
  g_free (oid1);
}

RUN_TEST (6, mongo_utils_oid_new);
