/* mongo-utils.h - libmongo-client utility functions
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

#ifndef LIBMONGO_CLIENT_UTILS_H
#define LIBMONGO_CLIENT_UTILS_H 1

#include <glib.h>

/** @defgroup mongo_util Mongo Utils
 *
 * Various utility functions related to MongoDB.
 *
 * @addtogroup mongo_util
 * @{
 */

/** Generate a new ObjectID.
 *
 * Based on the current time, pid, machine ID and a supplied sequence
 * number, generate a new ObjectID.
 *
 * The machine ID is generated upon first call, and is not changed
 * thereafter. The time and PID are checked at every invocation
 * however.
 *
 * @param seq is the sequence number to use.
 *
 * @note The ObjectID has space for only 24 bits of sequence bytes, so
 * it should be noted that while @a seq is 32 bits wide, only 24 of
 * that will be used.
 *
 * @returns A newly allocated ObjectID. Freeing it is the
 * responsibility of the caller.
 */
guint8 *mongo_util_oid_new (gint32 seq);

/** @} */

#endif
