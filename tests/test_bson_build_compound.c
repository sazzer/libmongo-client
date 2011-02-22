#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "bson.h"
#include "test.h"
#include "test-generator.h"

gboolean
test_bson_compound_flat (void)
{
  bson *b = test_bson_generate_flat ();

  return test_dump_add_bson (b);
}

gboolean
test_bson_compound_nested (void)
{
  bson *b = test_bson_generate_nested ();

  return test_dump_add_bson (b);
}

int
main (void)
{
  g_assert (test_dump_setup ());

  g_assert (test_bson_compound_flat ());
  g_assert (test_bson_compound_nested ());

  g_assert (test_dump_teardown ());

  return 0;
}
