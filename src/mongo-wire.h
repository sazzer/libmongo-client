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

/** Mongo packet header.
 *
 * Every mongo packet has a header like this. Normally, one does not
 * need to touch it, though.
 */
#pragma pack(1)
typedef struct
{
  gint32 length; /**< Full length of the packet, including the
		    header. */
  gint32 id; /**< Sequence ID, used when MongoDB responds to a
		command. */
  gint32 resp_to; /**< ID the response is an answer to. Only sent by
		     the MongoDB server, never set on client-side. */
  gint32 opcode; /**< The opcode of the command. @see
		    mongo_wire_opcode. <*/
} mongo_packet_header;

/** An opaque Mongo Packet on the wire.
 *
 * This structure contains the binary data that can be written
 * straight to the wire.
 */
typedef struct _mongo_packet mongo_packet;

/** Create an empty packet.
 *
 * Creates an empty packet to be filled in later with
 * mongo_wire_packet_set_header() and mongo_packet_set_data().
 *
 * @returns A newly allocated packet, or NULL on error.
 */
mongo_packet *mongo_wire_packet_new (void);

/** Get the header data of a packet.
 *
 * Retrieve the mongo packet's header data.
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
				     const mongo_packet_header **header);
/** Set the header data of a packet.
 *
 * Override the mongo packet's header data.
 *
 * @note No sanity checks are done, use this function with great care.
 *
 * @param p is the packet whose header we want to override.
 * @param header is the header structure to use.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_wire_packet_set_header (mongo_packet *p,
				       const mongo_packet_header *header);

/** Get the data part of a packet.
 *
 * Retrieve the raw binary blob of the mongo packet's data.
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
/** Set the data part of a packet.
 *
 * Overrides the data part of a packet, adjusting the packet length in
 * the header too.
 *
 * @note No sanity checks are performed on the data, it is the
 * caller's responsibility to supply valid information.
 *
 * @param p is the packet whose data is to be set.
 * @param data is the data to set.
 * @param size is the size of the data.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_wire_packet_set_data (mongo_packet *p, const guint8 *data,
				     gint32 size);

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
 * @param docs are the BSON documents to insert. One must close the
 * list with a NULL value.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 */
mongo_packet *mongo_wire_cmd_insert (gint32 id, const gchar *ns,
				     const bson *docs, ...);

/** Construct a query command.
 *
 * @param id is the sequence id.
 * @param ns is the namespace, the database and collection name
 * concatenaded, and separated with a single dot.
 * @param flags are the query options.
 * @param skip is the number of documents to skip.
 * @param ret is the number of documents to return.
 * @param query is the query BSON object.
 * @param sel is the (optional) selector BSON object indicating the
 * fields to return. Passing NULL will return all fields.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 *
 * @todo The flags should be handled better.
 */
mongo_packet *mongo_wire_cmd_query (gint32 id, const gchar *ns, gint32 flags,
				    gint32 skip, gint32 ret, const bson *query,
				    const bson *sel);

/** Construct a get more command.
 *
 * @param id is the sequence id.
 * @param ns is the namespace, the database and collection name
 * concatenaded, and separated with a single dot.
 * @param ret is the number of documents to return.
 * @param cursor_id is the ID of the cursor to use.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 */
mongo_packet *mongo_wire_cmd_get_more (gint32 id, const gchar *ns,
				       gint32 ret, gint64 cursor_id);

/** Construct a delete command.
 *
 * @param id is the sequence id.
 * @param ns is the namespace, the database and collection name
 * concatenaded, and separated with a single dot.
 * @param flags are the delete options.
 * @param sel is the BSON object to use as a selector.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 *
 * @todo flags should be easier to handle.
 */
mongo_packet *mongo_wire_cmd_delete (gint32 id, const gchar *ns,
				     gint32 flags, const bson *sel);

/** Construct a kill cursors command.
 *
 * @param id is the sequence id.
 * @param n is the number of cursors to delete.
 * @param cursor_ids are the ids of the cursors to delete.
 *
 * @note One must supply exaclty @a n number of cursor IDs.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 */
mongo_packet *mongo_wire_cmd_kill_cursors (gint32 id, gint32 n,
					   gint64 cursor_ids, ...);

/** Construct a custom command.
 *
 * Custom commands are queries run in the db.$cmd namespace. The
 * commands themselves are queries, and as such, BSON objects.
 *
 * @param id is the sequence id.
 * @param db is the database in which the command shall be run.
 * @param command is the BSON object representing the command.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 */
mongo_packet *mongo_wire_cmd_custom (gint32 id, const gchar *db,
				     const bson *command);

/** @} */
/** @} */
#endif