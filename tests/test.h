#ifndef LIBMONGO_CLIENT_TEST_H
#define LIBMONGO_CLIENT_TEST_H 1

#include "bson.h"
#include <stdio.h>

static gchar *current_test = NULL;

#define TEST(s) current_test = #s
#define PASS()					\
  {						\
    printf (" + %s\n", current_test);		\
    current_test = NULL;			\
  }
#define FAIL()					\
  {						\
    printf (" - %s\n", current_test);		\
    current_test = NULL;			\
    abort ();					\
  }
#define SKIP()					\
  {						\
    printf (" ! %s, skipped\n", current_test);	\
    current_test = NULL;			\
    return;					\
  }

gboolean dump_data (const guint8 *d, gint32 size);
gboolean dump_bson (bson *b);

#endif
