#ifndef LIBMONGO_CLIENT_TEST_H
#define LIBMONGO_CLIENT_TEST_H 1

#include "tap.h"
#include "bson.h"

#define RUN_TEST(n, t) \
  int		       \
  main (void)	       \
  {		       \
    plan (n);	       \
    test_##t ();       \
    return 0;	       \
  }

bson *test_bson_generate_full (void);

#endif
