/* mongo.h - libmongo-client general header
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

#include "bson.h"
#include "mongo-wire.h"
#include "mongo-client.h"
#include "mongo-utils.h"

/** @mainpage libmongo-client
 *
 * @section Introduction
 *
 * libmongo-client is an alternative MongoDB driver for the C
 * language, with clarity, correctness and completeness in mind.
 *
 * On these pages, one will find the complete API documentation.
 *
 * @section Structure
 *
 * The library can be split into four major parts:
 *   - bson: The low-level BSON implementation. @see bson_mod
 *   - mongo-wire: Functions to construct packets that can be sent
 *     later. @see mongo_wire
 *   - mongo-client: The high-level API that deals with the
 *     network. @see mongo_client
 *   - mongo-utils: Various miscellaneous utilities related to either
 *     BSON or MongoDB. @see mongo_util
 *
 * The intended way to use the library to work with MongoDB is to
 * first construct the BSON objects, then construct the packets, and
 * finally send the packets out to the network.
 *
 * The reason behind the split between packet construction and sending
 * is because we want to be able to construct packets in one thread
 * and send it in another, if so need be.
 *
 * This split also allows scenarios where the packet must be sent over
 * a medium the library was not prepared for (such as an SSL tunnel),
 * or when the packet is supposed to be sent to multiple destinations
 * (for whatever reason - perhaps for being logged to a file for
 * debugging purposes).
 *
 * All in all, this separation between modules provides a great deal
 * of flexibility.
 */
