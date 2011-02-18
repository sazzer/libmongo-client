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

/** @internal Mongo Connection state object. */
struct _mongo_connection
{
  gint fd; /**< The file descriptor associated with the connection. */
  gint32 request_id; /**< The last sent command's requestID. */
};

/** @internal Connect to a MongoDB server, using an existing connection object.
 *
 * Connects to a MongoDB server, but uses an existing connection
 * object to store the connection info in.
 *
 * @param host is the address of the server.
 * @param port is the port to connect to.
 * @param conn is a pointer to an allocated mongo_connection object.
 *
 * @returns The conn object, or NULL on error. Upon error, the
 * contents of the conn pointer are unspecified.
 */
mongo_connection *mongo_connection_new (const char *host, int port,
					mongo_connection **conn);

/** @internal Construct an insert command, using a va_list.
 *
 * @param id is the sequence id.
 * @param ns is the namespace, the database and collection name
 * concatenaded, and separated with a single dot.
 * @param ap is the stdarg list of BSON documents to insert,
 * terminated with a NULL value.
 *
 * @returns A newly allocated packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 */
mongo_packet *mongo_wire_cmd_insert_va (gint32 id, const gchar *ns,
					va_list ap);

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
