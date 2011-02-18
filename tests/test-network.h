#ifndef LIBMONGO_CLIENT_TEST_NETWORK_H
#define LIBMONGO_CLIENT_TEST_NETWORK_H

#include "test.h"
#include <glib.h>

extern gchar *TEST_SERVER_IP;
extern gchar *TEST_SERVER_HOST;
extern gchar *TEST_SERVER_IPV6;
extern gint TEST_SERVER_PORT;

extern gchar *TEST_SECONDARY_IP;
extern gint TEST_SECONDARY_PORT;

gboolean test_getenv_server (void);
gboolean test_getenv_server_extra (void);
gboolean test_getenv_secondary (void);
void test_env_free (void);

#endif
