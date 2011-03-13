#define _GNU_SOURCE

#include "test.h"
#include "bson.h"

#include <dlfcn.h>
#include <stdio.h>

static gint baa_null_at = -1;
static gint baa_cnt = 0;

static gint basn_null_at = -1;
static gint basn_cnt = 0;

static gint gtm0n_null_at = -1;
static gint gtm0n_cnt = 0;

GByteArray *
g_byte_array_append (GByteArray *array,
		     const guint8 *data,
		     guint len)
{
  SAVE_OLD_FUNC (g_byte_array_append);

  if (baa_null_at != -1 && baa_cnt >= baa_null_at)
    {
      baa_cnt = -1;
      return NULL;
    }
  baa_cnt++;

  return CALL_OLD_FUNC (g_byte_array_append, array, data, len);
}

GByteArray *
g_byte_array_sized_new (guint size)
{
  SAVE_OLD_FUNC (g_byte_array_sized_new);

  if (basn_null_at != -1 && basn_cnt >= basn_null_at)
    {
      basn_cnt = -1;
      return NULL;
    }
  basn_cnt++;

  return CALL_OLD_FUNC (g_byte_array_sized_new, size);
}

gpointer
g_try_malloc0_n (gsize n_blocks,
		 gsize n_block_bytes)
{
  SAVE_OLD_FUNC (g_try_malloc0_n);

  if (gtm0n_null_at != -1 && gtm0n_cnt >= gtm0n_null_at)
    {
      gtm0n_cnt = -1;
      return NULL;
    }
  gtm0n_cnt++;

  return CALL_OLD_FUNC (g_try_malloc0_n, n_blocks, n_block_bytes);
}

void
test_bson_new_oom (void)
{
  bson *b;

  gtm0n_null_at = 0;
  b = bson_new ();
  ok (b == NULL,
      "bson_new() & new0 OOM case handled properly");
  gtm0n_null_at = -1;
  gtm0n_cnt = 0;

  basn_null_at = 0;
  b = bson_new ();
  ok (b == NULL,
      "bson_new() & array_sized_new OOM case handled properly");
  basn_null_at = -1;
  basn_cnt = 0;

  baa_null_at = 0;
  b = bson_new ();
  ok (b == NULL,
      "bson_new() & array_append OOM case handled properly");
  baa_null_at = -1;
  baa_cnt = 0;
}

void
test_bson_new_from_data_oom (void)
{
  bson *o, *b;

  o = bson_new ();
  bson_append_boolean (o, "test", TRUE);
  bson_finish (o);

  gtm0n_null_at = 0;
  b = bson_new_from_data (bson_data (o), bson_size (o) - 1);
  gtm0n_null_at = -1;
  gtm0n_cnt = 0;
  ok (o && b == NULL,
      "bson_new_from_data() handles OOM");

  basn_null_at = 0;
  b = bson_new_from_data (bson_data (o), bson_size (o) - 1);
  basn_null_at = -1;
  basn_cnt = 0;
  ok (o && b == NULL,
      "bson_new_from_data() handles OOM");

  baa_null_at = 0;
  b = bson_new_from_data (bson_data (o), bson_size (o) - 1);
  baa_null_at = -1;
  baa_cnt = 0;
  ok (o && b == NULL,
      "bson_new_from_data() handles OOM");

  bson_free (o);
}

void
test_bson_finish_oom (void)
{
  bson *b;

  b = bson_new ();

  baa_null_at = 0;
  ok (b && bson_finish (b) == FALSE,
      "bson_finish() handles OOM properly");
  baa_null_at = -1;
  baa_cnt = 0;

  bson_free (b);
}

void
test_bson_append_binary_oom (void)
{
  bson *b;
  gboolean ret;

  b = bson_new ();
  baa_null_at = 0;
  ret = bson_append_binary (b, "binary", 2,
			    (guint8 *)"foobar", 6);
  baa_null_at = -1;
  baa_cnt = 0;
  ok (b && ret == FALSE,
      "bson_binary_append() handles OOM properly");
  bson_free (b);

  b = bson_new ();
  baa_null_at = 1;
  ret = bson_append_binary (b, "binary", 2,
			    (guint8 *)"foobar", 6);
  baa_null_at = -1;
  baa_cnt = 0;
  ok (b && ret == FALSE,
      "bson_binary_append() handles OOM properly");
  bson_free (b);

  b = bson_new ();
  baa_null_at = 2;
  ret = bson_append_binary (b, "binary", 2,
			    (guint8 *)"foobar", 6);
  baa_null_at = -1;
  baa_cnt = 0;
  ok (b && ret == FALSE,
      "bson_binary_append() handles OOM properly");
  bson_free (b);

  b = bson_new ();
  baa_null_at = 3;
  ret = bson_append_binary (b, "binary", 2,
			    (guint8 *)"foobar", 6);
  baa_null_at = -1;
  baa_cnt = 0;
  ok (b && ret == FALSE,
      "bson_binary_append() handles OOM properly");
  bson_free (b);

  b = bson_new ();
  baa_null_at = 4;
  ret = bson_append_binary (b, "binary", 2,
			    (guint8 *)"foobar", 6);
  baa_null_at = -1;
  baa_cnt = 0;
  ok (b && ret == FALSE,
      "bson_binary_append() handles OOM properly");
  bson_free (b);
}

void
test_bson_cursor_new_oom (void)
{
  bson *b;

  b = bson_new ();
  bson_finish (b);

  gtm0n_null_at = 0;
  ok (b && bson_cursor_new (b) == NULL,
      "bson_cursor_new() handles OOM properly");
  gtm0n_null_at = -1;
  gtm0n_cnt = 0;
  bson_free (b);
}

void
test_bson_find_oom (void)
{
  bson *b;

  b = bson_new ();
  bson_append_int32 (b, "i32", 1984);
  bson_finish (b);

  gtm0n_null_at = 0;
  ok (b && bson_find (b, "i32") == NULL,
      "bson_find() handles OOM properly");
  gtm0n_null_at = -1;
  gtm0n_cnt = 0;
  bson_free (b);
}

void
test_bson_append_string_element_oom (void)
{
  bson *b;
  gboolean ret;

  b = bson_new ();
  baa_null_at = 3;
  ret = bson_append_string (b, "str", "hello world", -1);
  baa_null_at = -1;
  baa_cnt = 1;
  ok (b && ret == FALSE,
      "bson_append_string() handles OOM");
  bson_free (b);

  b = bson_new ();
  baa_null_at = 4;
  ret = bson_append_string (b, "str", "hello world", -1);
  baa_null_at = -1;
  baa_cnt = 1;
  ok (b && ret == FALSE,
      "bson_append_string() handles OOM");
  bson_free (b);

  b = bson_new ();
  baa_null_at = 5;
  ret = bson_append_string (b, "str", "hello world", -1);
  baa_null_at = -1;
  baa_cnt = 0;
  ok (b && ret == FALSE,
      "bson_append_string() handles OOM");
  bson_free (b);
}

void
test_bson_oom (void)
{
  test_bson_new_oom ();
  test_bson_finish_oom ();
  test_bson_append_binary_oom ();
  test_bson_cursor_new_oom ();
  test_bson_find_oom ();
  test_bson_new_from_data_oom ();

  test_bson_append_string_element_oom ();
}

RUN_TEST (17, bson_oom);
