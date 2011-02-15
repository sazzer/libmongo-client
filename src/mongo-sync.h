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

#include "mongo-client.h"

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
gboolean mongo_sync_cmd_update (mongo_connection *conn,
				const gchar *ns,
				gint32 flags, const bson *selector,
				const bson *update);

/** Send an insert command to MongoDB.
 *
 * Constructs and sends an insert command to MongodB.
 *
 * @param conn is the connection to work with.
 * @param ns is the namespace to work in.
 * @param doc is the document to insert.
 *
 * @returns TRUE on success, FALSE otherwise.
 *
 * @note Unlike mongo_wire_cmd_insert(), the wrapper can only insert a
 * single document.
 */
gboolean mongo_sync_cmd_insert (mongo_connection *conn,
				const gchar *ns,
				const bson *doc);

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
mongo_packet *mongo_sync_cmd_query (mongo_connection *conn,
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
mongo_packet *mongo_sync_cmd_get_more (mongo_connection *conn,
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
gboolean mongo_sync_cmd_delete (mongo_connection *conn, const gchar *ns,
				gint32 flags, const bson *sel);

/** Send a kill_cursors command to MongoDB.
 *
 * @param conn is the connection to work with.
 * @param cursor_id is the cursor to kill.
 *
 * @note Unlike mongo_wire_cmd_kill_cursors(), this function can only
 * kill one cursors at a time.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_sync_cmd_kill_cursor (mongo_connection *conn,
				     gint64 cursor_id);

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
mongo_packet *mongo_sync_cmd_custom (mongo_connection *conn,
				     const gchar *db,
				     const bson *command);

gdouble mongo_sync_cmd_count (mongo_connection *conn,
			      const gchar *db, const gchar *coll,
			      const bson *query);

/** @} */

#endif
