#include "bson.h"
#include "test.h"
#include "test-generator.h"

#include <glib.h>

void
test_bson_from_data (void)
{
  bson *orig, *new;

  TEST (bson_from_data);
  orig = test_bson_generate_nested ();
  g_assert ((new = bson_new_from_data (bson_data (orig), bson_size (orig) - 1)));
  bson_finish (new);

  g_assert (orig != new);
  g_assert_cmpint (bson_size (orig), ==, bson_size (new));
  g_assert (bson_data (orig) != bson_data (new));

  bson_free (orig);
  bson_free (new);
  PASS ();
}

void
test_bson_reset (void)
{
  bson *b;

  TEST (bson_reset);
  b = test_bson_generate_nested ();
  bson_finish (b);

  g_assert_cmpint (bson_size (b), !=, -1);

  g_assert (bson_reset (b));
  g_assert (bson_finish (b));

  /* 5 is the size of an empty BSON object: 32bit length + zero byte
     ending. */
  g_assert_cmpint (bson_size (b), ==, 5);

  bson_free (b);
  PASS ();
}

int
main (void)
{
  PLAN (1, 2);

  test_bson_from_data ();
  test_bson_reset ();

  return 0;
}
