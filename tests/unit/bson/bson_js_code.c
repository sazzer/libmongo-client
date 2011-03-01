#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_js_code (void)
{
  bson *b;

  /* Test #1: A single JS element, with default size. */
  b = bson_new ();
  ok (bson_append_javascript (b, "hello",
			      "function () { print (\"hello world!\"); }", -1),
      "bson_append_javascript() works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 56, "BSON javascript element size check");
  ok (memcmp (bson_data (b),
	      "\070\000\000\000\015\150\145\154\154\157\000\050\000\000\000"
	      "\146\165\156\143\164\151\157\156\040\050\051\040\173\040\160"
	      "\162\151\156\164\040\050\042\150\145\154\154\157\040\167\157"
	      "\162\154\144\041\042\051\073\040\175\000\000",
	      bson_size (b)) == 0,
      "BSON javascript element contents check");
  bson_free (b);

  /* Test #2: A single javascript element, with explicit length. */
  b = bson_new ();
  ok (bson_append_javascript (b, "hello",
			      "print (\"hello world!\"); garbage is gone.",
			  strlen ("print (\"hello world!\");")),
      "bson_append_javascript() with explicit length works");
  bson_finish (b);

  cmp_ok (bson_size (b), "==", 40, "BSON javascript element size check, #2");
  ok (memcmp (bson_data (b),
	      "\050\000\000\000\015\150\145\154\154\157\000\030\000\000\000"
	      "\160\162\151\156\164\040\050\042\150\145\154\154\157\040\167"
	      "\157\162\154\144\041\042\051\073\000\000",
	      bson_size (b)) == 0,
      "BSON javascript element contents check, #2");
  bson_free (b);

  /* Test #3: Negative test, passing an invalid length. */
  b = bson_new ();
  ok (bson_append_javascript (b, "hello", "print();", -42) == FALSE,
      "bson_append_javascript() fails with an invalid length");
  bson_free (b);
}

RUN_TEST (7, bson_js_code);
