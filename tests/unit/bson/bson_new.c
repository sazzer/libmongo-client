#include "bson.h"
#include "test.h"
#include "tap.h"

#include <string.h>

void
test_bson_new (void)
{
  bson *b;

  ok ((b = bson_new ()) != NULL, "bson_new() works");
  ok (bson_data (b) == NULL,
      "bson_data() with an unfished object should fail");
  ok (bson_size (b) == -1,
      "bson_size() with an unfinished object should fail");
  ok (bson_finish (b), "bson_finish() works");
  ok (bson_finish (b),
      "bson_finish() works on an already finished object too");
  bson_free (b);

  ok (bson_size (NULL) == -1, "bson_size(NULL) works correctly");
  ok (bson_data (NULL) == NULL, "bson_data(NULL) works correctly");
  ok (bson_finish (NULL) == FALSE, "bson_finish(NULL) works correctly");
  bson_free (NULL);
}

RUN_TEST (8, bson_new);
