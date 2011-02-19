/* mongo-sync.h - libmongo-sync synchronous wrapper API
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

#ifndef LIBMONGO_SYNC_H
#define LIBMONGO_SYNC_H 1

#include <mongo-client.h>

#include <glib.h>

/** @defgroup mongo_sync Mongo Sync API
 *
 * These commands provide wrappers for the most often used MongoDB
 * commands. All of these will send the command, and receive any
 * results, thus saving the caller from having to do that himself.
 *
 * However, these are only of use when blocking the application is not
 * an issue. For asynchronous operation, one should still construct
 * the packets himself, and send / receive when appropriate.
 *
 * @addtogroup mongo_sync
 * @{
 */

/** Opaque synchronous connection object. */
typedef struct _mongo_sync_connection mongo_sync_connection;

/** Synchronously connect to a MongoDB server.
 *
 * Sets up a synchronous connection to a MongoDB server.
 *
 * @param host is the address of the server.
 * @param port is the port to connect to.
 * @param slaveok signals whether queries made against a slave are
 * acceptable.
 *
 * @returns A newly allocated mongo_sync_connection object, or NULL on
 * error. It is the responsibility of the caller to close and free the
 * connection when appropriate.
 */
mongo_sync_connection *mongo_sync_connect (const char *host,
					   int port,
					   gboolean slaveok);

/** Attempt to connect to the master of a replica set.
 *
 * Given an existing connection, this will determine the master node,
 * and attempt to connect there.
 *
 * @param conn is an existing MongoDB connection.
 *
 * @returns A mongo_sync_collection object, or NULL if the reconnect fails
 * for one reason or the other.
 *
 * @note If the existing connection is not the master, it will be
 * destroyed, whether the connection to the new master suceeds or not.
 */
mongo_sync_connection *mongo_sync_connect_to_master (mongo_sync_connection *conn);

/** Close and free a synchronous MongoDB connection.
 *
 * @param conn is the connection to close.
 *
 * @note The object will be freed, and shall not be used afterwards!
 */
void mongo_sync_disconnect (mongo_sync_connection *conn);

/** Retrieve the state of the SLAVE_OK flag from a sync connection.
 *
 * @param conn is the connection to check the flag on.
 *
 * @returns The state of the SLAVE_OK flag.
 */
gboolean mongo_sync_conn_get_slaveok (const mongo_sync_connection *conn);

/** Set the SLAVE_OK flag on a sync connection.
 *
 * @param conn is the connection to set the flag on.
 * @param slaveok is the state to set.
 */
void mongo_sync_conn_set_slaveok (mongo_sync_connection *conn,
				  gboolean slaveok);

/** Send an update command to MongoDB.
 *
 * Constructs and sends an update command to MongoDB.
 *
 * @param conn is the connection to work with.
 * @param ns is the namespace to work in.
 * @param flags are the flags for the update command. See
 * mongo_wire_cmd_update().
 * @param selector is the BSON document that will act as the selector.
 * @param update is the BSON document that contains the updated
 * values.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_sync_cmd_update (mongo_sync_connection *conn,
				const gchar *ns,
				gint32 flags, const bson *selector,
				const bson *update);

/** Send an insert command to MongoDB.
 *
 * Constructs and sends an insert command to MongodB.
 *
 * @param conn is the connection to work with.
 * @param ns is the namespace to work in.
 * @tparam docs are the documents to insert. One must close the list
 * with a NULL value.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_sync_cmd_insert (mongo_sync_connection *conn,
				const gchar *ns, ...);

/** Send a query command to MongoDB.
 *
 * @param conn is the connection to work with.
 * @param ns is the namespace, the database and collection name
 * concatenaded, and separated with a single dot.
 * @param flags are the query options. See mongo_wire_cmd_query().
 * @param skip is the number of documents to skip.
 * @param ret is the number of documents to return.
 * @param query is the query BSON object.
 * @param sel is the (optional) selector BSON object indicating the
 * fields to return. Passing NULL will return all fields.
 *
 * @returns A newly allocated reply packet, or NULL on error. It is the
 * responsibility of the caller to free the packet once it is not used
 * anymore.
 */
mongo_packet *mongo_sync_cmd_query (mongo_sync_connection *conn,
				    const gchar *ns, gint32 flags,
				    gint32 skip, gint32 ret, const bson *query,
				    const bson *sel);

/** Send a get more command to MongoDB.
 *
 * @param conn is the connection to work with.
 * @param ns is the namespace, the database and collection name
 * concatenaded, and separated with a single dot.
 * @param ret is the number of documents to return.
 * @param cursor_id is the ID of the cursor to use.
 *
 * @returns A newly allocated reply packet, or NULL on error. It is
 * the responsibility of the caller to free the packet once it is not
 * used anymore.
 */
mongo_packet *mongo_sync_cmd_get_more (mongo_sync_connection *conn,
				       const gchar *ns,
				       gint32 ret, gint64 cursor_id);

/** Send a delete command to MongoDB.
 *
 * @param conn is the connection to work with.
 * @param ns is the namespace, the database and collection name
 * concatenaded, and separated with a single dot.
 * @param flags are the delete options. See mongo_wire_cmd_delete().
 * @param sel is the BSON object to use as a selector.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_sync_cmd_delete (mongo_sync_connection *conn, const gchar *ns,
				gint32 flags, const bson *sel);

/** Send a kill_cursors command to MongoDB.
 *
 * @param conn is the connection to work with.
 * @param n is the number of cursors to kill.
 * @tparam cursor_ids is the list of cursor ids to kill.
 *
 * @note One must supply exaclty @a n number of cursor IDs.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_sync_cmd_kill_cursors (mongo_sync_connection *conn,
				      gint32 n, ...);

/** Send a custom command to MongoDB.
 *
 * Custom commands are queries run in the db.$cmd namespace. The
 * commands themselves are queries, and as such, BSON objects.
 *
 * @param conn is the connection to work with.
 * @param db is the database in which the command shall be run.
 * @param command is the BSON object representing the command.
 *
 * @returns A newly allocated reply packet, or NULL on error. It is
 * the responsibility of the caller to free the packet once it is not
 * used anymore.
 */
mongo_packet *mongo_sync_cmd_custom (mongo_sync_connection *conn,
				     const gchar *db,
				     const bson *command);

/** Send a count() command to MongoDB.
 *
 * The count command is an efficient way to count tha available
 * documents matching a selector.
 *
 * @param conn is the connection to work with.
 * @param db is the name of the database.
 * @param coll is the name of the collection.
 * @param query is the optional selector (NULL will count all
 * documents within the collection).
 *
 * @returns The number of matching documents, or -1 on error.
 */
gdouble mongo_sync_cmd_count (mongo_sync_connection *conn,
			      const gchar *db, const gchar *coll,
			      const bson *query);

/** Send a drop() command to MongoDB.
 *
 * With this command, one can easily drop a collection.
 *
 * @param conn is the connection to work with.
 * @param db is the name of the database.
 * @param coll is the name of the collection to drop.
 *
 * @returns TRUE if the collection was dropped, FALSE otherwise.
 */
gboolean mongo_sync_cmd_drop (mongo_sync_connection *conn,
			      const gchar *db, const gchar *coll);

/** Get the last error from MongoDB.
 *
 * Retrieves the last error from MongoDB.
 *
 * @param conn is the connection to work with.
 * @param db is the name of the database.
 * @param error is a pointer to a string variable that will hold the
 * error message.
 *
 * @returns TRUE if the error was succesfully retrieved, FALSE
 * otherwise. The output variable @a error is only set if the function
 * is returning TRUE.
 */
gboolean mongo_sync_cmd_get_last_error (mongo_sync_connection *conn,
					const gchar *db, gchar **error);

/** Reset the last error variable in MongoDB.
 *
 * @param conn is the connection to work with.
 * @param db is the name of the database.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_sync_cmd_reset_error (mongo_sync_connection *conn,
				     const gchar *db);

/** Check whether the current node is the master.
 *
 * @param conn is the connection to work with.
 *
 * @returns TRUE if it is master, FALSE otherwise and on errors.
 */
gboolean mongo_sync_cmd_is_master (mongo_sync_connection *conn);

/** @} */

#endif
