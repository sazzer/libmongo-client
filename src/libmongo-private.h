/* libmongo-private.h - private headers for libmongo-client
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

/** @file libmongo-private.h
 *
 * Private types and functions, for internal use in libmongo-client only.
 */

#include "mongo.h"

/** @internal Mongo Connection state object. */
struct _mongo_connection
{
  gint fd; /**< The file descriptor associated with the connection. */
  gint32 request_id; /**< The last sent command's requestID. */
};

/** @internal Synchronous connection object. */
struct _mongo_sync_connection
{
  mongo_connection super; /**< The parent object. */
  gboolean slaveok; /**< Whether queries against slave nodes are
		       acceptable. */
  gboolean safe_mode; /**< Safe-mode signal flag. */
  gboolean auto_reconnect; /**< Auto-reconnect flag. */

  /** Replica Set properties. */
  struct
  {
    GList *seeds; /**< Replica set seeds, as a list of strings. */
    GList *hosts; /**< Replica set members, as a list of strings. */
    gchar *primary; /**< The replica master, if any. */
  } rs;

  gchar *last_error; /**< The last error from the server, caught
			during queries. */
  gint32 max_insert_size; /**< Maximum number of bytes an insert
			     command can be before being split to
			     smaller chunks. Used for bulk inserts. */
};

/** @internal Synchronous pool connection object. */
struct _mongo_sync_pool_connection
{
  mongo_sync_connection super; /**< The parent object. */

  gint pool_id; /**< ID of the connection. */
  gboolean in_use; /**< Whether the object is in use or not. */
};

/** @internal Construct a kill cursors command, using a va_list.
 *
 * @param id is the sequence id.
 * @param n is the number of cursors to delete.
 * @param ap is the va_list of cursors to kill.
 *
 * @note One must supply exaclty @a n number of cursor IDs.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 */
mongo_packet *mongo_wire_cmd_kill_cursors_va (gint32 id, gint32 n,
					      va_list ap);

/** @internal Get the header data of a packet, without conversion.
 *
 * Retrieve the mongo packet's header data, but do not convert the
 * values from little-endian. Use only when the source has the data in
 * the right byte order already.
 *
 * @param p is the packet which header we seek.
 * @param header is a pointer to a variable which will hold the data.
 *
 * @note Allocating the @a header is the responsibility of the caller.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean
mongo_wire_packet_get_header_raw (const mongo_packet *p,
				  mongo_packet_header *header);

/** @internal Set the header data of a packet, without conversion.
 *
 * Override the mongo packet's header data, but do not convert the
 * values from little-endian. Use only when the source has the data in
 * the right byte order already.
 *
 * @note No sanity checks are done, use this function with great care.
 *
 * @param p is the packet whose header we want to override.
 * @param header is the header structure to use.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean
mongo_wire_packet_set_header_raw (mongo_packet *p,
				  const mongo_packet_header *header);
