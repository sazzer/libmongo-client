#include "bson.h"
#include "test.h"
#include "tap.h"
#include "test-generator.h"

#include <string.h>

void
test_bson_new_from_data (void)
{
  bson *orig, *new;

  orig = test_bson_generate_nested ();
  bson_finish (orig);

  ok (bson_new_from_data (NULL, 0) == NULL,
      "bson_new_from_data (NULL, 0) fails");
  ok (bson_new_from_data (NULL, bson_size (orig)) == NULL,
      "bson_new_from_data (NULL, size) fails");
  ok (bson_new_from_data (bson_data (orig), 0) == NULL,
      "bson_new_from_data (orig, 0) fails");
  ok (bson_new_from_data (bson_data (orig), -1) == NULL,
      "bson_new_from_data (orig, -1) fails");
  ok (bson_new_from_data (NULL, -1) == NULL,
      "bson_new_from_data (NULL, -1) fails");

  ok ((new = bson_new_from_data (bson_data (orig),
				 bson_size (orig) - 1)) != NULL,
      "bson_new_from_data() works");
  cmp_ok (bson_size (new), "==", -1,
	  "Copied object is unfinished");
  bson_finish (new);

  ok (orig != new, "Copied BSON object is not the same as the original");

  cmp_ok (bson_size (orig), "==", bson_size (new),
	  "Copied (& finished) object has the same size as the original");
  ok (bson_data (orig) != bson_data (new),
      "The copied data is not the same as the original");
  ok (memcmp (bson_data (orig), bson_data (new),
	      bson_size (orig)) == 0,
      "The copied data is identical to the original");

  bson_free (orig);
  bson_free (new);
}

RUN_TEST (11, bson_new_from_data);
