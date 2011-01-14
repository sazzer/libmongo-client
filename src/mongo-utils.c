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

guint8 *
mongo_util_oid_new (gint32 seq)
{
  guint8 *oid;
  time_t t = GINT32_TO_BE (time (NULL));
  pid_t pid = getpid ();
  gint16 short_pid;
  gint32 mid;
  gint32 tmp = GINT32_TO_BE (seq);

  if (!machine_id)
    {
      srand (t);
      machine_id = rand ();
    }
  mid = machine_id;

  /*
   * If our pid has more than 16 bits, let half the bits modulate the
   * machine_id.
   */
  if (sizeof (pid_t) > 2)
    {
      mid ^= pid >> 16;
    }
  short_pid = (gint16)pid;

  oid = (guint8 *)g_try_new0 (guint8, 12);

  /* Sequence number, last 3 bytes
   * For simplicity's sake, we put this in first, and overwrite the
   * first byte later.
   */
  memcpy (oid + 4 + 2 + 2, &tmp, 4);
  /* First four bytes: the time, BE byte order */
  memcpy (oid, &t, 4);
  /* Machine ID, byte order doesn't matter, 3 bytes */
  memcpy (oid + 4, &mid, 3);
  /* PID, byte order doesn't matter, 2 bytes */
  memcpy (oid + 4 + 3, &short_pid, 2);

  return oid;
}
