#include "bson.h"
#include "test.h"
#include "tap.h"
#include "test-generator.h"

void
test_bson_reset (void)
{
  bson *b;

  b = test_bson_generate_nested ();
  bson_finish (b);

  cmp_ok (bson_size (b), "!=", -1,
	  "bson_size() != -1 on a non-empty document");
  ok (bson_reset (b), "bson_reset() works");
  cmp_ok (bson_size (b), "==", -1,
	  "bson_size() on a reseted object returns an error");
  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "bson_size() on a reseted & finished object matches the "
	  "size of an empty document");
  bson_free (b);
}

RUN_TEST (4, bson_reset);
