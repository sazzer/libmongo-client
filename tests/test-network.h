#ifndef LIBMONGO_CLIENT_TEST_NETWORK_H
#define LIBMONGO_CLIENT_TEST_NETWORK_H

#include "test.h"

#ifndef TEST_SERVER_IP
#define TEST_SERVER_IP "127.0.0.1"
#endif

#ifndef TEST_SERVER_IPV6
#define TEST_SERVER_IPV6 "::1"
#endif

#ifndef TEST_SERVER_HOST
#define TEST_SERVER_HOST "localhost"
#endif

#ifndef TEST_SERVER_PORT
#define TEST_SERVER_PORT 27017
#endif

#ifndef TEST_SECONDARY_IP
#define TEST_SECONDARY_IP "127.0.0.1"
#endif

#ifndef TEST_SECONDARY_PORT
#define TEST_SECONDARY_PORT 27018
#endif

#endif
