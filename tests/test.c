#include "test.h"

#include <glib.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int dump_fd = -1;

void
ignore_sigpipe (void)
{
  struct sigaction sa;

  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGPIPE, &sa, NULL);
}

gboolean
test_dump_setup (void)
{
  int fd;

  fd = open ("libmongo-tests.bson", O_RDWR | O_CREAT | O_TRUNC, 0600);
  if (fd == -1)
    return FALSE;
  dump_fd = fd;
  return TRUE;
}

gboolean
test_dump_teardown (void)
{
  return (close (dump_fd) == 0);
}

gboolean
test_dump_add_bson (const bson *b)
{
  if (dump_fd == -1)
    return FALSE;
  if (write (dump_fd, bson_data (b), bson_size (b)) != bson_size (b))
    return FALSE;
  return TRUE;
}
