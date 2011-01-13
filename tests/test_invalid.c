#include "test.h"
#include "bson.h"
#include "mongo_wire.h"

#include "test-generator.h"

#include <glib.h>

void
test_invalid_bson_null ()
{
  bson *b;

  TEST (bson_invalid_null);
  g_assert (bson_size (NULL) == -1);
  g_assert (bson_data (NULL) == NULL);
  g_assert (bson_finish (NULL) == FALSE);
  bson_free (NULL);
  PASS ();

  g_assert ((b = bson_new ()));

  TEST (bson_invalid_double_params);
  g_assert (bson_append_double (b, NULL, 3.14) == FALSE);
  PASS ();

  TEST (bson_invalid_string_params);
  g_assert (bson_append_string (b, NULL, "str", -1) == FALSE);
  g_assert (bson_append_string (b, "test", NULL, -1) == FALSE);
  g_assert (bson_append_string (b, "test", NULL, 0) == FALSE);
  g_assert (bson_append_string (b, "test", "string", 0) == FALSE);
  PASS ();

  TEST (bson_invalid_document_params);
  g_assert (bson_append_document (b, NULL, NULL) == FALSE);
  g_assert (bson_append_document (b, "test", b) == FALSE);
  PASS ();

  TEST (bson_invalid_oid_params);
  g_assert (bson_append_oid (b, NULL, NULL) == FALSE);
  g_assert (bson_append_oid (b, "_id", NULL) == FALSE);
  PASS ();

  TEST (bson_invalid_regex_params);
  g_assert (bson_append_regex (b, NULL, NULL, NULL) == FALSE);
  g_assert (bson_append_regex (b, "regex", NULL, NULL) == FALSE);
  g_assert (bson_append_regex (b, "regex", "/1234/", NULL) == FALSE);
  g_assert (bson_append_regex (b, "regex", NULL, "i") == FALSE);
  PASS ();

  bson_free (b);

  TEST (bson_invalid_from_data);
  g_assert ((b = bson_new_from_data (NULL, 0)) == NULL);
  g_assert ((b = bson_new_from_data ((guint8 *)"data", 0)) == NULL);
  g_assert ((b = bson_new_from_data ((guint8 *)"data", -1)) == NULL);
  PASS ();
}

void
test_invalid_bson_cursor ()
{
  bson *b;
  bson_cursor *c;

  b = bson_new ();
  g_assert (bson_append_string (b, "hello", "world", -1));

  TEST (bson_invalid_cursor_new_null);
  g_assert ((c = bson_cursor_new (NULL)) == NULL);
  PASS ();

  TEST (bson_invalid_cursor_next);
  g_assert (bson_cursor_next (NULL) == FALSE);
  PASS ();

  TEST (bson_invalid_cursor_find_params);
  g_assert (bson_find (NULL, NULL) == FALSE);
  g_assert ((c = bson_find (b, NULL)) == FALSE);
  g_assert ((c = bson_find (b, "hello")) == FALSE);
  bson_finish (b);
  g_assert ((c = bson_find (b, "hello")));
  PASS ();

  g_free (c);

  TEST (bson_invalid_cursor_type);
  g_assert ((c = bson_cursor_new (b)));
  g_assert_cmpint (bson_cursor_type (c), ==, BSON_TYPE_NONE);
  PASS ();

  TEST (bson_invalid_cursor_key);
  g_assert (bson_cursor_key (NULL) == NULL);
  g_assert (bson_cursor_key (c) == NULL);
  PASS ();

  TEST (bson_invalid_cursor_get);
  g_assert (bson_cursor_get_string (NULL, NULL) == FALSE);
  g_assert (bson_cursor_get_string (c, NULL) == FALSE);
  g_assert (bson_cursor_next (c));
  g_assert (bson_cursor_get_string (c, NULL) == FALSE);
  PASS ();

  g_free (c);
  bson_free (b);
}

int
main (void)
{
  test_invalid_bson_null ();
  test_invalid_bson_cursor ();
}
