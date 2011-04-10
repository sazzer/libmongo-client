#include "tap.h"
#include "test.h"
#include "mongo-utils.h"

void
test_mongo_utils_oid_init (void)
{
  mongo_util_oid_init (0);
  mongo_util_oid_init (1234);

  /* We don't do any real testing here, only check if it does not
     crash. To verify that it works, we need to create a new OID, and
     that will be tested by other unit tests.
  */
  ok (TRUE,
      "mongo_util_oid_init() does not crash.");
}

RUN_TEST (1, mongo_utils_oid_init);
