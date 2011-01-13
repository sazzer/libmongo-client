/* mongo-wire.h - libmongo-client's MongoDB wire protocoll implementation.
 * Copyright 2011 Gergely Nagy <algernon@balabit.hu>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
