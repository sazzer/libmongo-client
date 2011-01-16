/* mongo-utils.c - libmongo-client utility functions
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

#include <glib.h>

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static guint32 machine_id = 0;
static gint16 pid = 0;

void
mongo_util_oid_init (gint32 mid)
{
  pid_t p = getpid ();

  if (mid == 0)
    {
      srand (time (NULL));
      machine_id = rand ();
    }
  else
    machine_id = mid;

  /*
   * If our pid has more than 16 bits, let half the bits modulate the
   * machine_id.
   */
  if (sizeof (pid_t) > 2)
    {
      machine_id ^= pid >> 16;
    }
  pid = (gint16)p;
}

guint8 *
mongo_util_oid_new_with_time (gint32 ts, gint32 seq)
{
  guint8 *oid;
  time_t t = GINT32_TO_BE (ts);
  gint32 tmp = GINT32_TO_BE (seq);

  if (machine_id == 0 || pid == 0)
    return NULL;

  oid = (guint8 *)g_try_new0 (guint8, 12);
  if (!oid)
    return NULL;

  /* Sequence number, last 3 bytes
   * For simplicity's sake, we put this in first, and overwrite the
   * first byte later.
   */
  memcpy (oid + 4 + 2 + 2, &tmp, 4);
  /* First four bytes: the time, BE byte order */
  memcpy (oid, &t, 4);
  /* Machine ID, byte order doesn't matter, 3 bytes */
  memcpy (oid + 4, &machine_id, 3);
  /* PID, byte order doesn't matter, 2 bytes */
  memcpy (oid + 4 + 3, &pid, 2);

  return oid;
}

guint8 *
mongo_util_oid_new (gint32 seq)
{
  return mongo_util_oid_new_with_time (time (NULL), seq);
}
