#include "test.h"
#include "tap.h"
#include "mongo-wire.h"

#include <string.h>

void
test_mongo_wire_cmd_insert_n (void)
{
  bson *ins, *tmp;
  const bson *docs[10];
  mongo_packet *p;

  mongo_packet_header hdr;
  const guint8 *data;
  gint32 data_size;

  bson_cursor *c;
  gint32 pos;

  ins = test_bson_generate_full ();
  tmp = bson_new ();

  docs[0] = ins;
  docs[1] = tmp;
  docs[2] = ins;
  docs[3] = ins;
  docs[4] = NULL;
  docs[5] = ins;

  ok (mongo_wire_cmd_insert_n (1, NULL, 1, docs) == NULL,
      "mongo_wire_cmd_insert_n() fails with a NULL namespace");
  ok (mongo_wire_cmd_insert_n (1, "test.ns", 1, NULL) == NULL,
      "mongo_wire_cmd_insert_n() fails with no documents");
  ok (mongo_wire_cmd_insert_n (1, "test.ns", 0, docs) == NULL,
      "mongo_wire_cmd_insert_n() fails with no documents");
  ok (mongo_wire_cmd_insert_n (1, "test.ns", 2, docs) == NULL,
      "mongo_wire_cmd_insert_n() fails with an unfinished document");
  bson_finish (tmp);
  ok (mongo_wire_cmd_insert_n (1, "test.ns", 5, docs) == NULL,
      "mongo_wire_cmd_insert_n() fails with a NULL document in the array");
  ok ((p = mongo_wire_cmd_insert_n (1, "test.ns", 3, docs)) != NULL,
      "mongo_wire_cmd_insert() works");
  bson_free (ins);
  bson_free (tmp);

  /* Test basic header data */
  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size appears fine");

  cmp_ok (hdr.length, "==", sizeof (mongo_packet_header) + data_size,
	  "Packet header length is correct");
  cmp_ok (hdr.id, "==", 1, "Header ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Response ID is ok");

  /*
   * Test the first document
   */

  /* pos = zero + collection_name + NULL */
  pos = sizeof (gint32) + strlen ("test.ns") + 1;
  ok ((ins = bson_new_from_data (data + pos,
				 _DOC_SIZE (data, pos) - 1)) != NULL,
      "First document is included");
  bson_finish (ins);

  ok ((c = bson_find (ins, "int32")) != NULL,
      "BSON contains 'int32'");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT32,
	  "int32 has correct type");
  bson_cursor_next (c);
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT64,
	  "next element has correct type too");
  ok (bson_cursor_next (c) == FALSE,
      "No more data after the update BSON object");
  bson_cursor_free (c);

  /*
   * Test the second document
   */
  pos += bson_size (ins);
  ok ((tmp = bson_new_from_data (data + pos,
				 _DOC_SIZE (data, pos) - 1)) != NULL,
      "Second document is included");
  bson_finish (tmp);
  cmp_ok (bson_size (tmp), "==", 5,
	  "Second document is empty");

  bson_free (ins);
  bson_free (tmp);
  mongo_wire_packet_free (p);
}

RUN_TEST (17, mongo_wire_cmd_insert_n);
