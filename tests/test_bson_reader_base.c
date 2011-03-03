#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "bson.h"
#include "test.h"
#include "test-generator.h"

static gboolean
test_bson_reader_huge (void)
{
  bson *b, *s;
  bson_cursor *c;
  gchar buffer[32768];
  gint32 ds1, ds2;

  TEST(bson_reader_huge);

  memset (buffer, 'a', sizeof (buffer));
  buffer[sizeof(buffer) - 1] = '\0';
  b = bson_new ();
  g_assert (bson_append_int32 (b, "preamble", 1));
  g_assert (bson_append_string (b, "huge", buffer, -1));
  g_assert (bson_append_int32 (b, "post", 1234));
  bson_finish (b);

  ds1 = bson_size (b);

  s = bson_new ();
  g_assert (bson_append_document (s, "hugedoc", b));
  bson_finish (s);
  bson_free (b);

  g_assert_cmpint (bson_size (s), >, ds1);

  c = bson_find (s, "hugedoc");
  bson_cursor_get_document (c, &b);

  ds2 = bson_size (b);
  g_assert_cmpint (ds1, ==, ds2);

  bson_cursor_free (c);
  bson_free (b);
  bson_free (s);
  PASS();

  return TRUE;
}

int
main (void)
{
  PLAN (1, 23);

  g_assert (test_bson_reader_flat ());
  g_assert (test_bson_reader_nested ());
  g_assert (test_bson_reader_regexp ());
  g_assert (test_bson_reader_binary ());
  g_assert (test_bson_reader_huge ());

  return 0;
}
