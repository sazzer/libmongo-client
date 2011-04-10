#include "tap.h"
#include "test.h"
#include "bson.h"

#include <string.h>

void
test_bson_cursor_get_javascript_w_scope (void)
{
  bson *b, *scope = NULL, *valid;
  bson_cursor *c;
  const gchar *s = "deadbeef";

  ok (bson_cursor_get_javascript_w_scope (NULL, &s, &scope) == FALSE,
      "bson_cursor_get_javascript_w_scope() with a NULL cursor fails");

  b = test_bson_generate_full ();
  c = bson_cursor_new (b);

  ok (bson_cursor_get_javascript_w_scope (c, NULL, &scope) == FALSE,
      "bson_cursor_get_javascript_w_scope() with a NULL js destination fails");
  ok (bson_cursor_get_javascript_w_scope (c, &s, NULL) == FALSE,
      "bson_cursor_get_javascript_w_scope() with a NULL scope destinatin fails");
  ok (bson_cursor_get_javascript_w_scope (c, &s, &scope) == FALSE,
      "bson_cursor_get_javascript_w_scope() at the initial position fails");
  is (s, "deadbeef",
      "destination remains unchanged after failed cursor operations");
  bson_cursor_free (c);

  c = bson_find (b, "print");
  ok (bson_cursor_get_javascript_w_scope (c, &s, &scope),
      "bson_cursor_get_javascript_w_scope() works");
  is (s, "alert (v);",
      "bson_cursor_get_javascript_w_scope() returns the correct result");

  valid = bson_new ();
  bson_append_string (valid, "v", "hello world", -1);
  bson_finish (valid);

  cmp_ok (bson_size (scope), "==", bson_size (valid),
	  "The returned scope's length is correct");
  ok (memcmp (bson_data (scope), bson_data (valid),
	      bson_size (scope)) == 0,
      "The returned scope is correct");
  bson_free (valid);

  bson_cursor_next (c);
  ok (bson_cursor_get_javascript_w_scope (c, &s, &scope) == FALSE,
      "bson_cursor_get_javascript_w_scope() should fail when the cursor "
      "points to non-javascript data");

  bson_cursor_free (c);
  bson_free (b);
  bson_free (scope);
}

RUN_TEST (10, bson_cursor_get_javascript_w_scope);
