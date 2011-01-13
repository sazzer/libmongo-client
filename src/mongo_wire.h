#ifndef LIBMONGO_CLIENT_MONGO_WIRE_H
#define LIBMONGO_CLIENT_MONGO_WIRE_H 1

#include <glib.h>

#include "bson.h"

typedef struct _mongo_packet mongo_packet;

gint32 mongo_wire_packet_get_header (const mongo_packet *p,
				     const guint8 **header);
gint32 mongo_wire_packet_get_data (const mongo_packet *p, const guint8 **data);

void mongo_wire_packet_free (mongo_packet *p);

mongo_packet *mongo_wire_cmd_update (gint32 id, const gchar *ns,
				     gint32 flags, const bson *selector,
				     const bson *update);
mongo_packet * mongo_wire_cmd_insert (gint32 id, const gchar *ns,
				      const bson *doc);


#endif
