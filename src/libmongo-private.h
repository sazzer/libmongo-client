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
