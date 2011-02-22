#ifndef LIBMONGO_CLIENT_TEST_H
#define LIBMONGO_CLIENT_TEST_H 1

#include "bson.h"
#include <stdio.h>
#include <stdlib.h>

static gchar *current_test __attribute__((unused)) = NULL;
static gint current_test_no __attribute__((unused)) = 0;

#ifndef TEST_SERVER_DB
#define TEST_SERVER_DB "test"
#endif

#ifndef TEST_SERVER_COLLECTION
#define TEST_SERVER_COLLECTION "libmongo"
#endif

#define TEST_SERVER_NS TEST_SERVER_DB "." TEST_SERVER_COLLECTION

#define PLAN(b,e) {				\
    printf ("%d..%d\n", b, e);			\
  }

#define TEST(s)					\
  {						\
    current_test = #s;				\
    current_test_no++;				\
    printf ("# %s\n", current_test);		\
  }
#define PASS()					\
  {						\
    printf ("ok %d\n", current_test_no);	\
    current_test = NULL;			\
  }
#define SKIP(s)						\
  {							\
    printf ("ok %d # SKIP %s\n", current_test_no, s);	\
    current_test = NULL;				\
  }

#define SKIP_ALL(s)				\
  {						\
    printf ("1..0 # SKIP %s\n", s);		\
    exit (0);					\
  }

void ignore_sigpipe (void);

gboolean test_dump_setup (void);
gboolean test_dump_add_bson (const bson *b);
gboolean test_dump_teardown (void);

#endif
