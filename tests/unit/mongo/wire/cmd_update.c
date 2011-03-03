#include "test.h"
#include "tap.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_cmd_update (void)
{
  bson *sel, *upd, *tmp;
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  bson_cursor *c;
  gint32 pos;

  sel = bson_new ();
  bson_append_null (sel, "_id");
  bson_finish (sel);

  upd = test_bson_generate_full ();

  ok (mongo_wire_cmd_update (1, NULL, 0, sel, upd) == NULL,
      "mongo_wire_cmd_update() with a NULL namespace should fail");
  ok (mongo_wire_cmd_update (1, "test.ns", 0, NULL, upd) == NULL,
      "mongo_wire_cmd_update() with a NULL selector should fail");
  ok (mongo_wire_cmd_update (1, "test.ns", 0, sel, NULL) == NULL,
      "mongo_wire_cmd_update() with a NULL update should fail");

  tmp = bson_new ();
  ok (mongo_wire_cmd_update (1, "test.ns", 0, tmp, upd) == NULL,
      "mongo_wire_cmd_update() fails with an unfinished selector");
  ok (mongo_wire_cmd_update (1, "test.ns", 0, sel, tmp) == NULL,
      "mongo_wire_cmd_update() fails with an unfinished update");
  bson_free (tmp);

  ok ((p = mongo_wire_cmd_update (1, "test.ns", 0, sel, upd)) != NULL,
      "mongo_wire_cmd_update() works");

  bson_free (sel);

  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size looks fine");
  cmp_ok (hdr.length, "==", sizeof (mongo_packet_header) + data_size,
	  "Packet header length is OK");
  cmp_ok (hdr.id, "==", 1, "Packet request ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Packet reply ID is ok");

  /*
   * Verify the selector object.
   */

  /* pos = zero + collection_name + NULL + flags */
  pos = sizeof (gint32) + strlen ("test.ns") + 1 + sizeof (gint32);
  ok ((sel = bson_new_from_data (data + pos, (gint32)data[pos] - 1)) != NULL,
      "Packet contains a valid BSON selector document");
  bson_finish (sel);

  ok ((c = bson_find (sel, "_id")) != NULL,
      "BSON contains an _id");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_NULL,
	  "_id has correct type");
  bson_cursor_free (c);
  bson_free (sel);

  /*
   * Verify the update object
   */
  pos += (gint32)data[pos];
  ok ((tmp = bson_new_from_data (data + pos,
				 _DOC_SIZE (data, pos) - 1)) != NULL,
      "Packet contains a valid BSON update document");
  bson_finish (tmp);
  cmp_ok (bson_size (upd), "==", bson_size (tmp),
	  "Packet's update document has the correct size");

  ok ((c = bson_find (tmp, "int32")) != NULL,
      "BSON contains 'int32'");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT32,
	  "int32 has correct type");
  bson_cursor_next (c);
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT64,
	  "next element has correct type too");
  ok (bson_cursor_next (c) == FALSE,
      "No more data after the update BSON object");

  bson_cursor_free (c);
  bson_free (tmp);
  bson_free (upd);
  mongo_wire_packet_free (p);
}

RUN_TEST (19, mongo_wire_cmd_update);
