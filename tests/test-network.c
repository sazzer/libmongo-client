#include "test-network.h"
#include "mongo-utils.h"

#include <stdlib.h>
#include <string.h>

gchar *TEST_SERVER_IP;
gchar *TEST_SERVER_HOST;
gchar *TEST_SERVER_IPV6;
gint TEST_SERVER_PORT;

gchar *TEST_SECONDARY_IP;
gint TEST_SECONDARY_PORT;

gboolean
test_getenv_server (void)
{
  gchar *addr = getenv ("TEST_SERVER");

  return mongo_util_parse_addr (addr, &TEST_SERVER_IP, &TEST_SERVER_PORT);
}

gboolean
test_getenv_server_extra (void)
{
  gchar *host = getenv ("TEST_SERVER_HOST");
  gchar *ipv6 = getenv ("TEST_SERVER_IPV6");

  if (!host && !ipv6)
    return FALSE;

  if (host)
    TEST_SERVER_HOST = g_strdup (host);
  if (ipv6)
    TEST_SERVER_IPV6 = g_strdup (ipv6);

  return TRUE;
}

gboolean
test_getenv_secondary (void)
{
  gchar *addr = getenv ("TEST_SECONDARY");

  return mongo_util_parse_addr (addr, &TEST_SECONDARY_IP,
				&TEST_SECONDARY_PORT);
}

void
test_env_free (void)
{
  g_free (TEST_SERVER_IP);
  g_free (TEST_SERVER_HOST);
  g_free (TEST_SERVER_IPV6);

  g_free (TEST_SECONDARY_IP);
}
