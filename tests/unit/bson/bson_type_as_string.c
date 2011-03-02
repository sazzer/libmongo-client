#include "bson.h"
#include "test.h"
#include "tap.h"

#include <string.h>

#define CHECK_TYPE(t)					\
  is (bson_type_as_string (t), #t,			\
      "bson_type_as_string(%s) works", #t)

void
test_bson_type_as_string (void)
{
  CHECK_TYPE (BSON_TYPE_NONE);
  CHECK_TYPE (BSON_TYPE_DOUBLE);
  CHECK_TYPE (BSON_TYPE_STRING);
  CHECK_TYPE (BSON_TYPE_DOCUMENT);
  CHECK_TYPE (BSON_TYPE_ARRAY);
  CHECK_TYPE (BSON_TYPE_BINARY);
  CHECK_TYPE (BSON_TYPE_UNDEFINED);
  CHECK_TYPE (BSON_TYPE_OID);
  CHECK_TYPE (BSON_TYPE_BOOLEAN);
  CHECK_TYPE (BSON_TYPE_UTC_DATETIME);
  CHECK_TYPE (BSON_TYPE_NULL);
  CHECK_TYPE (BSON_TYPE_REGEXP);
  CHECK_TYPE (BSON_TYPE_DBPOINTER);
  CHECK_TYPE (BSON_TYPE_JS_CODE);
  CHECK_TYPE (BSON_TYPE_SYMBOL);
  CHECK_TYPE (BSON_TYPE_JS_CODE_W_SCOPE);
  CHECK_TYPE (BSON_TYPE_INT32);
  CHECK_TYPE (BSON_TYPE_TIMESTAMP);
  CHECK_TYPE (BSON_TYPE_INT64);
  CHECK_TYPE (BSON_TYPE_MIN);
  CHECK_TYPE (BSON_TYPE_MAX);

  ok (bson_type_as_string (42) == NULL,
      "bson_type_as_string() returns NULL on invalid type.");
}

RUN_TEST (22, bson_type_as_string);
