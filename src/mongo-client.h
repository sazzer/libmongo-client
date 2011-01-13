#ifndef LIBMONGO_CLIENT_H
#define LIBMONGO_CLIENT_H 1

#include "bson.h"
#include "mongo_wire.h"

#include <glib.h>

gint mongo_connect (const char *host, int port);
void mongo_disconnect (gint fd);

gboolean mongo_packet_send (gint fd, const mongo_packet *p);

#endif
