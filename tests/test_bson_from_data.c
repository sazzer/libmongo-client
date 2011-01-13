#include "bson.h"
#include "test.h"
#include "test-generator.h"

#include <glib.h>

void
test_bson_from_data (void)
{
  bson *orig, *new;

  orig = test_bson_generate_nested ();
  g_assert ((new = bson_new_from_data (bson_data (orig), bson_size (orig) - 1)));
  bson_finish (new);

  g_assert (orig != new);
  g_assert_cmpint (bson_size (orig), ==, bson_size (new));
  g_assert (bson_data (orig) != bson_data (new));

  bson_free (orig);
  bson_free (new);
}

int
main (void)
{
  test_bson_from_data ();

  return 0;
}
