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

/** @defgroup mongo_wire Mongo Wire Protocol
 *
 * The structures and functions within this module implement the
 * MongoDB wire protocol: functions to assemble various commands into
 * binary blobs that can be sent over the wire.
 *
 * @see mongo_client
 *
 * @addtogroup mongo_wire
 * @{
 */

/** @defgroup mongo_wire_packet Packets
 *
 * @addtogroup mongo_wire_packet
 * @{
 */

/** An opaque Mongo Packet on the wire.
 *
 * This structure contains the binary data that can be written
 * straight to the wire.
 */
typedef struct _mongo_packet mongo_packet;

/** Get the header data of a packet.
 *
 * Retrieve the raw binary blog of the mongo packet's header.
 *
 * @param p is the packet which header we seek.
 * @param header is a pointer to a variable which will hold the data.
 *
 * @note The @a header parameter will point to an internal structure,
 * which shall not be freed or written to.
 *
 * @returns The size of the header, or -1 on error.
 */
gint32 mongo_wire_packet_get_header (const mongo_packet *p,
				     const guint8 **header);
/** Get the data part of a packet.
 *
 * Retrieve the raw binary blog of the mongo packet's data.
 *
 * @param p is the packet which header we seek.
 * @param data is a pointer to a variable which will hold the data.
 *
 * @note The @a data parameter will point to an internal structure,
 * which shall not be freed or written to.
 *
 * @returns The size of the data, or -1 on error.
 */
gint32 mongo_wire_packet_get_data (const mongo_packet *p, const guint8 **data);
/** Free up a mongo packet.
 *
 * @param p is the packet to free.
 *
 * @note The packet shall not be used afterwards.
 */
void mongo_wire_packet_free (mongo_packet *p);

/** @} */

/** @defgroup mongo_wire_cmd Commands
 *
 * Each command has an @a id parameter, which can be used to track
 * replies to various commands. It is the responsibility of the caller
 * to keep track of IDs.
 *
 * @addtogroup mongo_wire_cmd
 * @{
 */

/** Construct an update command.
 *
 * @param id is the sequence id.
 * @param ns is the namespace, the database and collection name
 * concatenated, and separated with a single dot.
 * @param flags are the flags for the update command.
 * @param selector is the BSON document that will act as the selector.
 * @param update is the BSON document that contains the updated values.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 *
 * @todo The flags should be handled better, we should have an enum
 * for them, whose values can be OR'd together.
 */
mongo_packet *mongo_wire_cmd_update (gint32 id, const gchar *ns,
				     gint32 flags, const bson *selector,
				     const bson *update);
/** Construct an insert command.
 *
 * @param id is the sequence id.
 * @param ns is the namespace, the database and collection name
 * concatenaded, and separated with a single dot.
 * @param doc is the BSON document to insert.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 */
mongo_packet * mongo_wire_cmd_insert (gint32 id, const gchar *ns,
				      const bson *doc);

/** @} */
/** @} */
#endif
