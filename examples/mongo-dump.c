#include "mongo.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct
{
  gchar *host;
  gint port;
  gchar *db;
  gchar *coll;
  gchar *output;
  gchar *ns;
  gboolean verbose;
  gboolean slaveok;
  gboolean master_sync;
} config_t;

#define VLOG(...) { if (config->verbose) fprintf (stderr, __VA_ARGS__); }

gdouble
mongo_dump_packet (config_t *config, mongo_packet *p, gdouble pos, gdouble cnt,
		   int fd)
{
  gint32 i;
  mongo_reply_packet_header rh;

  mongo_wire_reply_packet_get_header (p, &rh);

  for (i = 1; i <= rh.returned; i++)
    {
      bson *b;

      mongo_wire_reply_packet_get_nth_document (p, i, &b);
      bson_finish (b);

#if 0
      VLOG ("\rDumping object %.0f/%.0f, size=%d", pos + i, cnt,
	    bson_size (b));
      if (config->verbose)
	fflush (stderr);
#endif

      write (fd, bson_data (b), bson_size (b));
      bson_free (b);
    }
  VLOG("\r");

  return pos + i - 1;
}

int
mongo_dump (config_t *config)
{
  mongo_sync_connection *conn;
  bson *b;
  int fd;

  mongo_packet *p;
  mongo_reply_packet_header rh;
  gint64 cid;
  gdouble cnt, pos = 0;

  VLOG ("Connecting to %s:%d/%s.%s...\n", config->host, config->port,
	config->db, config->coll);

  conn = mongo_sync_connect (config->host, config->port, config->slaveok);
  if (!conn)
    {
      fprintf (stderr, "Error connecting to %s:%d: %s\n", config->host,
	       config->port, strerror (errno));
      exit (1);
    }

  if (config->master_sync)
    {
      VLOG ("Syncing to master...\n");
      conn = mongo_sync_reconnect (conn, TRUE);
      if (!conn)
	{
	  fprintf (stderr, "Error reconnecting to the master of %s:%d: %s\n",
		   config->host, config->port, strerror (errno));
	  exit (1);
	}
    }

  VLOG ("Counting documents...\n");
  cnt = mongo_sync_cmd_count (conn, config->db, config->coll, NULL);
  if (cnt < 0)
    {
      fprintf (stderr, "Error counting documents in %s.%s: %s\n",
	       config->db, config->coll, strerror (errno));
      mongo_sync_disconnect (conn);
      exit (1);
    }

  VLOG ("Opening output file '%s'...\n", config->output);
  if (strcmp (config->output, "-") == 0)
    fd = 1;
  else
    {
      fd = open (config->output, O_RDWR | O_CREAT | O_TRUNC, 0600);
      if (fd == -1)
	{
	  fprintf (stderr, "Error opening output file '%s': %s\n",
		   config->output, strerror (errno));
	  mongo_sync_disconnect (conn);
	  exit (1);
	}
    }

  VLOG ("Launching initial query...\n");
  b = bson_new ();
  bson_finish (b);
  p = mongo_sync_cmd_query (conn, config->ns,
			    MONGO_WIRE_FLAG_QUERY_NO_CURSOR_TIMEOUT,
			    0, 10, b, NULL);
  bson_free (b);
  if (!p)
    {
      unlink (config->output);
      close (fd);
      fprintf (stderr, "Error retrieving the cursor: %s\n",
	       strerror (errno));
      mongo_sync_disconnect (conn);
      exit (1);
    }

  mongo_wire_reply_packet_get_header (p, &rh);
  cid = rh.cursor_id;
  pos = mongo_dump_packet (config, p, pos, cnt, fd);
  mongo_wire_packet_free (p);

  while (pos < cnt)
    {
      gdouble pr = (pos + 10) / cnt;

      VLOG ("\rDumping... %03.2f%%", ((pr > 1) ? 1 : pr) * 100);
      if (config->verbose)
	fflush (stderr);

      p = mongo_sync_cmd_get_more (conn, config->ns, 10, cid);
      if (!p)
	{
	  unlink (config->output);
	  close (fd);
	  fprintf (stderr, "Error advancing the cursor: %s\n",
		   strerror (errno));
	  mongo_sync_disconnect (conn);
	  exit (1);
	}
      pos = mongo_dump_packet (config, p, pos, cnt, fd);
      mongo_wire_packet_free (p);
    }

  close (fd);
  mongo_sync_disconnect (conn);

  return 0;
}

int
main (int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context;
  config_t config = {
    NULL, 27017, NULL, NULL, NULL, NULL, FALSE, FALSE, FALSE
  };

  GOptionEntry entries[] =
    {
      { "host", 'h', 0, G_OPTION_ARG_STRING, &config.host,
	"Host to connect to", "HOST" },
      { "port", 'p', 0, G_OPTION_ARG_INT, &config.port, "Port", "PORT" },
      { "db", 'd', 0, G_OPTION_ARG_STRING, &config.db, "Database", "DB" },
      { "collection", 'c', 0, G_OPTION_ARG_STRING, &config.coll, "CONN" },
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &config.verbose,
	"Be verbose", NULL },
      { "output", 'o', 0, G_OPTION_ARG_STRING, &config.output,
	"Output", "FILENAME" },
      { "slave-ok", 's', 0, G_OPTION_ARG_NONE, &config.slaveok,
	"Connecting to slaves is ok", NULL },
      { "master-sync", 'm', 0, G_OPTION_ARG_NONE, &config.master_sync,
	"Reconnect to the replica master", NULL },
      { NULL }
    };

  context = g_option_context_new ("- dump a complete mongo collection");
  g_option_context_add_main_entries (context, entries, "mongo-dump");
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("option parsing failed: %s\n", error->message);
      exit (1);
    }

  if (!config.host || !config.port || !config.db ||
      !config.coll || !config.output)
    {
      gchar *help = g_option_context_get_help (context, TRUE, NULL);

      printf ("%s", help);
      exit (1);
    }

  config.ns = g_strdup_printf ("%s.%s", config.db, config.coll);
  return mongo_dump (&config);
}
