#include "test.h"
#include "tap.h"
#include "mongo-wire.h"
#include "mongo-client.h"

#include <errno.h>
#include "libmongo-private.h"

void
test_mongo_packet_send (void)
{
  mongo_packet *p;
  mongo_connection conn;
  mongo_packet_header h;

  p = mongo_wire_cmd_kill_cursors (1, 2, (gint64)3, (gint64)4);
  conn.fd = -1;

  ok (mongo_packet_send (NULL, p) == FALSE,
      "mongo_packet_send() fails with a NULL connection");
  ok (errno == ENOTCONN,
      "mongo_packet_send() with a NULL connection sets errno to ENOTCONN");
  ok (mongo_packet_send (&conn, NULL) == FALSE,
      "mongo_packet_send() fails with a NULL packet");
  ok (errno == EINVAL,
      "mongo_packet_send() with a NULL packet sets errno to EINVAL");
  ok (mongo_packet_send (&conn, p) == FALSE,
      "mongo_packet_send() fails if the FD is less than zero");
  ok (errno == EBADF,
      "mongo_packet_send() sets errno to EBADF is the FD is bad");
  mongo_wire_packet_free (p);

  p = mongo_wire_packet_new ();

  h.id = 42;
  h.resp_to = 0;
  h.opcode = 1;
  h.length = sizeof (mongo_packet_header);
  mongo_wire_packet_set_header (p, &h);

  conn.fd = 1;
  ok (mongo_packet_send (&conn, p) == FALSE,
      "mongo_packet_send() fails with an unfinished packet");

  mongo_wire_packet_free (p);
}

RUN_TEST (7, mongo_packet_send);
