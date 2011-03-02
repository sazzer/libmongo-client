#ifndef LIBMONGO_CLIENT_TEST_GENERATOR_H
#define LIBMONGO_CLIENT_TEST_GENERATOR_H 1

#include "bson.h"

bson *test_bson_generate_flat (void);
bson *test_bson_generate_nested (void);

#endif
