#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_document (void)
{
  bson *b, *e1, *e2;

  e1 = bson_new ();
  bson_append_int32 (e1, "i32", 1984);
  bson_append_string (e1, "str", "hello world", -1);
  bson_finish (e1);

  e2 = bson_new ();
  bson_append_string (e2, "foo", "bar", -1);
  ok (bson_append_document (e2, "subd", e1),
      "bson_append_document() works");
  bson_finish (e2);
  bson_free (e1);

  b = bson_new ();
  ok (bson_append_document (b, "doc", e2),
      "bson_append_document() works still");
  bson_finish (b);
  bson_free (e2);

  cmp_ok (bson_size (b), "==", 69, "BSON document element size check");
  ok (memcmp (bson_data (b),
	      "\105\000\000\000\003\144\157\143\000\073\000\000\000\002\146"
	      "\157\157\000\004\000\000\000\142\141\162\000\003\163\165\142"
	      "\144\000\043\000\000\000\020\151\063\062\000\300\007\000\000"
	      "\002\163\164\162\000\014\000\000\000\150\145\154\154\157\040"
	      "\167\157\162\154\144\000\000\000\000",
	      bson_size (b)) == 0,
      "BSON document element contents check");

  bson_free (b);

  e1 = bson_new ();
  bson_append_int32 (e1, "foo", 42);
  b = bson_new ();

  ok (bson_append_document (b, "doc", e1) == FALSE,
      "bson_append_document() with an unfinished document should fail");
  bson_finish (e1);
  ok (bson_append_document (b, NULL, e1) == FALSE,
      "bson_append_document() with a NULL key should fail");
  ok (bson_append_document (b, "doc", NULL) == FALSE,
      "bson_append_document() with a NULL document should fail");
  ok (bson_append_document (NULL, "doc", e1) == FALSE,
      "bson_append_document() without a BSON object should fail");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 5,
	  "BSON object should be empty");

  bson_free (e1);
  bson_free (b);
}

RUN_TEST (9, bson_document);
