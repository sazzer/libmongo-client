#include "test.h"
#include "tap.h"
#include "mongo-client.h"

#include <errno.h>

void
test_mongo_connect (void)
{
  /* We do not want to use the network here, which would be needed
     for a complete unit test. Therefore, we'll only test the
     failiure cases.
  */

  ok (mongo_connect (NULL, 27010) == NULL,
      "mongo_connect() fails with a NULL host");
  ok (errno == EINVAL,
      "mongo_connect() should fail with EINVAL if host is NULL");

  mongo_disconnect (NULL);
  ok (errno == ENOTCONN,
      "mongo_disconnect() should fail with ENOTCONN with a NULL connection");
}

RUN_TEST (3, mongo_connect);
