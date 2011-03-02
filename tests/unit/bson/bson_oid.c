#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_oid (void)
{
  bson *b;
  guint8 oid[] = "1234567890ab";

  b = bson_new ();
  ok (bson_append_oid (b, "_id", oid), "bson_append_oid() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 22, "BSON OID element size check");
  ok (memcmp (bson_data (b),
	      "\026\000\000\000\007\137\151\144\000\061\062\063\064\065\066"
	      "\067\070\071\060\141\142\000",
	      bson_size (b)) == 0,
      "BSON OID element contents check");

  bson_free (b);

  b = bson_new ();
  ok (bson_append_oid (b, "_id", NULL) == FALSE,
      "bson_append_oid() should fail without an OID");
  ok (bson_append_oid (b, NULL, oid) == FALSE,
      "bson_append_oid() should fail without a key name");
  ok (bson_append_oid (NULL, "_id", oid) == FALSE,
      "bson_append_oid() should fail without a BSON object");
  bson_finish (b);
  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");
  bson_free (b);
}

RUN_TEST (7, bson_oid);
