#include "test-network.h"

#include <stdlib.h>
#include <string.h>

gchar *TEST_SERVER_IP;
gchar *TEST_SERVER_HOST;
gchar *TEST_SERVER_IPV6;
gint TEST_SERVER_PORT;

gchar *TEST_SECONDARY_IP;
gint TEST_SECONDARY_PORT;

static gboolean
_parse_addr (const gchar *addr, gchar **host, gint *port)
{
  gchar *port_s, *ep;

  if (!addr)
    {
      *host = NULL;
      *port = -1;
      return FALSE;
    }

  /* Split up to host:port */
  port_s = g_strrstr (addr, ":");
  if (!port_s)
    return FALSE;
  port_s++;
  *host = g_strndup (addr, port_s - addr - 1);

  *port = strtol (port_s, &ep, 10);
  if (*port == LONG_MIN || *port == LONG_MAX)
    {
      g_free (*host);
      *host = NULL;
      *port = -1;
      return FALSE;
    }
  if (ep && *ep)
    {
      g_free (*host);
      *host = NULL;
      *port = -1;
      return FALSE;
    }
  return TRUE;
}

gboolean
test_getenv_server (void)
{
  gchar *addr = getenv ("TEST_SERVER");

  return _parse_addr (addr, &TEST_SERVER_IP, &TEST_SERVER_PORT);
}

gboolean
test_getenv_server_extra (void)
{
  gchar *host = getenv ("TEST_SERVER_HOST");
  gchar *ipv6 = getenv ("TEST_SERVER_IPV6");

  if (!host || !ipv6)
    return FALSE;

  TEST_SERVER_HOST = g_strdup (host);
  TEST_SERVER_IPV6 = g_strdup (ipv6);

  return TRUE;
}

gboolean
test_getenv_secondary (void)
{
  gchar *addr = getenv ("TEST_SECONDARY");

  return _parse_addr (addr, &TEST_SECONDARY_IP, &TEST_SECONDARY_PORT);
}

void
test_env_free (void)
{
  g_free (TEST_SERVER_IP);
  g_free (TEST_SERVER_HOST);
  g_free (TEST_SERVER_IPV6);

  g_free (TEST_SECONDARY_IP);
}
