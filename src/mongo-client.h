/* mongo-client.h - libmongo-client user API
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

#ifndef LIBMONGO_CLIENT_H
#define LIBMONGO_CLIENT_H 1

#include "bson.h"
#include "mongo-wire.h"

#include <glib.h>

/** @defgroup mongo_client Mongo Client
 *
 * @addtogroup mongo_client
 * @{
 */

/** Connect to a MongoDB server.
 *
 * Connects to a single MongoDB server.
 *
 * @param host is the IP address of the server.
 * @param port is the port to connect to.
 *
 * @note The @a host must be an IP address in dotted decimal form.
 *
 * @returns The file descriptor associated with the connection, or -1
 * on error.
 */
gint mongo_connect (const char *host, int port);
/** Disconnect from a MongoDB server.
 *
 * @param fd is the file descriptior associated with the connection.
 */
void mongo_disconnect (gint fd);

/** Sends an assembled command packet to MongoDB.
 *
 * @param fd is the file descriptor to send the packet to.
 * @param p is the packet to send.
 *
 * @returns TRUE on success, when the whole packet was sent, FALSE
 * otherwise.
 */
gboolean mongo_packet_send (gint fd, const mongo_packet *p);

/** @} */

#endif
