/*
 * vinagre-bookmarks.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008  Jonh Wendell <wendell@bani.com.br>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include "vinagre-bookmarks.h"
#include "vinagre-bookmarks-entry.h"
#include "vinagre-bookmarks-migration.h"

struct _VinagreBookmarksPrivate
{
  gchar        *filename;
  GSList       *entries, *conns;
  GFileMonitor *monitor;
};

enum
{
  BOOKMARK_CHANGED,
  LAST_SIGNAL
};

G_DEFINE_TYPE (VinagreBookmarks, vinagre_bookmarks, G_TYPE_OBJECT);

static VinagreBookmarks *book_singleton = NULL;
static guint signals[LAST_SIGNAL] = { 0 };

/* Prototypes */
static void vinagre_bookmarks_update_from_file (VinagreBookmarks *book);
static void vinagre_bookmarks_file_changed     (GFileMonitor     *monitor,
					        GFile             *file,
					        GFile             *other_file,
					        GFileMonitorEvent  event_type,
					        VinagreBookmarks  *book);

static void
vinagre_bookmarks_init (VinagreBookmarks *book)
{
  GFile *gfile;

  book->priv = G_TYPE_INSTANCE_GET_PRIVATE (book, VINAGRE_TYPE_BOOKMARKS, VinagreBookmarksPrivate);
  book->priv->entries = NULL;
  book->priv->filename = g_build_filename (g_get_user_data_dir (),
			                   "vinagre",
			                   VINAGRE_BOOKMARKS_FILE,
			                   NULL);

  if (!g_file_test (book->priv->filename, G_FILE_TEST_EXISTS))
    vinagre_bookmarks_migration_migrate (book->priv->filename);

  vinagre_bookmarks_update_from_file (book);

  gfile = g_file_new_for_path (book->priv->filename);
  book->priv->monitor = g_file_monitor_file (gfile,
                                             G_FILE_MONITOR_NONE,
                                             NULL,
                                             NULL);
  g_object_unref (gfile);

  g_signal_connect (book->priv->monitor,
                    "changed",
                    G_CALLBACK (vinagre_bookmarks_file_changed),
                    book);
}

static void
vinagre_bookmarks_clear_entries (VinagreBookmarks *book)
{
  g_slist_foreach (book->priv->entries, (GFunc) g_object_unref, NULL);
  g_slist_free (book->priv->entries);

  book->priv->entries = NULL;
}

static void
vinagre_bookmarks_finalize (GObject *object)
{
  VinagreBookmarks *book = VINAGRE_BOOKMARKS (object);

  g_free (book->priv->filename);
  book->priv->filename = NULL;

  G_OBJECT_CLASS (vinagre_bookmarks_parent_class)->finalize (object);
}

static void
vinagre_bookmarks_dispose (GObject *object)
{
  VinagreBookmarks *book = VINAGRE_BOOKMARKS (object);

  if (book->priv->entries)
    vinagre_bookmarks_clear_entries (book);

  if (book->priv->monitor)
    {
      g_file_monitor_cancel (book->priv->monitor);
      g_object_unref (book->priv->monitor);
      book->priv->monitor = NULL;
    }

  G_OBJECT_CLASS (vinagre_bookmarks_parent_class)->dispose (object);
}

static void
vinagre_bookmarks_class_init (VinagreBookmarksClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreBookmarksPrivate));

  object_class->finalize = vinagre_bookmarks_finalize;
  object_class->dispose  = vinagre_bookmarks_dispose;

  signals[BOOKMARK_CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (VinagreBookmarksClass, changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
}

static VinagreConnection *
find_conn_by_host (GSList *entries, const gchar *host, gint port)
{
  GSList *l;
  VinagreConnection *conn;

  for (l = entries; l; l = l->next)
    {
      VinagreBookmarksEntry *entry = VINAGRE_BOOKMARKS_ENTRY (l->data);

      switch (vinagre_bookmarks_entry_get_node (entry))
	{
	  case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	    if ((conn = find_conn_by_host (vinagre_bookmarks_entry_get_children (entry),
					  host,
					  port)))
	      return conn;
	    break;

	  case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
	    conn = vinagre_bookmarks_entry_get_conn (entry);
	    if ( (g_str_equal (host, vinagre_connection_get_host (conn))) &&
		 (port == vinagre_connection_get_port (conn) ) )
	      return g_object_ref (conn);
	    break;

	  default:
	    g_assert_not_reached ();
	}
    }
  return NULL;
}

VinagreConnection *
vinagre_bookmarks_exists (VinagreBookmarks *book,
                          const gchar *host,
                          gint port)
{
  VinagreConnection *conn = NULL;
  GSList *l, *next;

  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS (book), NULL);
  g_return_val_if_fail (host != NULL, NULL);

  return find_conn_by_host (book->priv->entries, host, port);
}

static void
vinagre_bookmarks_save_fill_xml (GSList *list, xmlTextWriter *writer)
{
  GSList                *l;
  VinagreBookmarksEntry *entry;
  VinagreConnection     *conn;

  for (l = list; l; l = l->next)
    {
      entry = (VinagreBookmarksEntry *) l->data;
      switch (vinagre_bookmarks_entry_get_node (entry))
	{
	  case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	    xmlTextWriterStartElement (writer, "folder");
	    xmlTextWriterWriteAttribute (writer, "name", vinagre_bookmarks_entry_get_name (entry));

	    vinagre_bookmarks_save_fill_xml (vinagre_bookmarks_entry_get_children (entry), writer);
	    xmlTextWriterEndElement (writer);
	    break;

	  case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
	    conn = vinagre_bookmarks_entry_get_conn (entry);

	    xmlTextWriterStartElement (writer, "item");
	    xmlTextWriterWriteElement (writer, "name", vinagre_connection_get_name (conn));
	    xmlTextWriterWriteElement (writer, "host", vinagre_connection_get_host (conn));
	    xmlTextWriterWriteFormatElement (writer, "port", "%d", vinagre_connection_get_port (conn));
	    xmlTextWriterWriteFormatElement (writer, "view_only", "%d", vinagre_connection_get_view_only (conn));
	    xmlTextWriterWriteFormatElement (writer, "scaling", "%d", vinagre_connection_get_scaling (conn));
	    xmlTextWriterWriteFormatElement (writer, "fullscreen", "%d", vinagre_connection_get_fullscreen (conn));

	    xmlTextWriterEndElement (writer);
	    break;

	  default:
	    g_assert_not_reached ();
	}
    }
}

static gboolean
vinagre_bookmarks_parse_boolean (const gchar* value)
{
  if (g_ascii_strcasecmp (value, "true") == 0 || strcmp (value, "1") == 0)
    return TRUE;

  return FALSE;
}

static VinagreBookmarksEntry *
vinagre_bookmarks_parse_item (xmlNode *root)
{
  VinagreBookmarksEntry *entry = NULL;
  VinagreConnection     *conn;
  xmlNode               *curr;
  xmlChar               *s_value;

  conn = vinagre_connection_new ();

  for (curr = root->children; curr; curr = curr->next)
    {
      s_value = xmlNodeGetContent (curr);

      if (!xmlStrcmp(curr->name, (const xmlChar *)"host"))
	vinagre_connection_set_host (conn, s_value);
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"name"))
	vinagre_connection_set_name (conn, s_value);
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"port"))
	vinagre_connection_set_port (conn, atoi (s_value));
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"view_only"))
	vinagre_connection_set_view_only (conn, vinagre_bookmarks_parse_boolean (s_value));
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"scaling"))
	vinagre_connection_set_scaling (conn, vinagre_bookmarks_parse_boolean (s_value));
      else if (!xmlStrcmp(curr->name, (const xmlChar *)"fullscreen"))
	vinagre_connection_set_fullscreen (conn, vinagre_bookmarks_parse_boolean (s_value));

      xmlFree (s_value);
    }

  if (vinagre_connection_get_port (conn) <= 0)
    vinagre_connection_set_port (conn, 5900);

  if (vinagre_connection_get_host (conn))
    entry = vinagre_bookmarks_entry_new_conn (conn);

  g_object_unref (conn);

  return entry;
}

static void
vinagre_bookmarks_parse_xml (VinagreBookmarks *book, xmlNode *root, VinagreBookmarksEntry *parent_entry)
{
  xmlNode *curr;
  xmlChar *folder_name;
  VinagreBookmarksEntry *entry;

  for (curr = root; curr; curr = curr->next)
    {
      if (curr->type == XML_ELEMENT_NODE)
	{
	  if (!xmlStrcmp(curr->name, (const xmlChar *)"folder"))
	    {
	      folder_name = xmlGetProp (curr, (const xmlChar *)"name");
	      if (folder_name && *folder_name)
		{
		  entry = vinagre_bookmarks_entry_new_folder ((const gchar *) folder_name);
		  if (parent_entry)
		    vinagre_bookmarks_entry_add_child (parent_entry, entry);
		  else
		    book->priv->entries = g_slist_insert_sorted (book->priv->entries,
							         entry,
							        (GCompareFunc)vinagre_bookmarks_entry_compare);

		  vinagre_bookmarks_parse_xml (book, curr->children, entry);
		}
	      xmlFree (folder_name);
	    }
	  else if (!xmlStrcmp(curr->name, (const xmlChar *)"item"))
	    {
	      entry = vinagre_bookmarks_parse_item (curr);
	      if (entry)
		{
		  if (parent_entry)
		    vinagre_bookmarks_entry_add_child (parent_entry, entry);
		  else
		    book->priv->entries = g_slist_insert_sorted (book->priv->entries,
								 entry,
								 (GCompareFunc)vinagre_bookmarks_entry_compare);
		}
	    }
	}
    }

}

static void
vinagre_bookmarks_update_from_file (VinagreBookmarks *book)
{
  xmlErrorPtr error;
  xmlNodePtr  root;
  xmlDocPtr   doc;

  if (!g_file_test (book->priv->filename, G_FILE_TEST_EXISTS))
    return;

  doc = xmlReadFile (book->priv->filename, NULL, XML_PARSE_NOERROR);

  if (!doc)
    {
      error = xmlGetLastError ();
      g_warning (_("Error while initializing bookmarks: %s"), error?error->message: _("Unknown error"));
      return;
    }

  root = xmlDocGetRootElement (doc);
  if (!root)
    {
      g_warning (_("Error while initializing bookmarks: The file seems to be empty"));
      xmlFreeDoc (doc);
      return;
    }

  if (xmlStrcmp (root->name, (const xmlChar *) "vinagre-bookmarks"))
    {
      g_warning (_("Error while initializing bookmarks: The file is not a vinagre bookmarks file"));
      xmlFreeDoc (doc);
      return;
    }

  vinagre_bookmarks_clear_entries (book);
  vinagre_bookmarks_parse_xml (book, root->xmlChildrenNode, NULL);
  xmlFreeDoc (doc);
}


static void
vinagre_bookmarks_file_changed (GFileMonitor      *monitor,
		                GFile             *file,
		                GFile             *other_file,
		                GFileMonitorEvent  event_type,
		                VinagreBookmarks  *book)
{
  if (event_type != G_FILE_MONITOR_EVENT_CHANGED &&
      event_type != G_FILE_MONITOR_EVENT_CREATED &&
      event_type != G_FILE_MONITOR_EVENT_DELETED)
    return;

  vinagre_bookmarks_update_from_file (book);

  g_signal_emit (book, signals[BOOKMARK_CHANGED], 0);
}

/* Public API */

VinagreBookmarks *
vinagre_bookmarks_get_default (void)
{
  if (G_UNLIKELY (!book_singleton))
    book_singleton = VINAGRE_BOOKMARKS (g_object_new (VINAGRE_TYPE_BOOKMARKS,
                                                      NULL));
  return book_singleton;
}

GSList *
vinagre_bookmarks_get_all (VinagreBookmarks *book)
{
  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS (book), NULL);

  return book->priv->entries;
}

void
vinagre_bookmarks_save_to_file (VinagreBookmarks *book)
{
  xmlTextWriter *writer;
  xmlBuffer     *buf;
  int            rc;
  GError        *error;

  writer = NULL;
  buf    = NULL;
  error  = NULL;

  buf = xmlBufferCreate ();
  if (!buf)
    {
      g_warning (_("Error while saving bookmarks: Failed to create the XML structure"));
      return;
    }

  writer = xmlNewTextWriterMemory(buf, 0);
  if (!writer)
    {
      g_warning (_("Error while saving bookmarks: Failed to create the XML structure"));
      goto finalize;
    }

  rc = xmlTextWriterStartDocument (writer, NULL, "utf-8", NULL);
  if (rc < 0)
    {
      g_warning (_("Error while saving bookmarks: Failed to initialize the XML structure"));
      goto finalize;
    }

  rc = xmlTextWriterStartElement (writer, "vinagre-bookmarks");
  if (rc < 0)
    {
      g_warning (_("Error while saving bookmarks: Failed to initialize the XML structure"));
      goto finalize;
    }

  vinagre_bookmarks_save_fill_xml (book->priv->entries, writer);

  rc = xmlTextWriterEndDocument (writer);
  if (rc < 0)
    {
      g_warning (_("Error while saving bookmarks: Failed to finalize the XML structure"));
      goto finalize;
    }

  if (!g_file_set_contents (book->priv->filename,
			    (const char *) buf->content,
			    -1,
			    &error))
    {
      g_warning (_("Error while saving bookmarks: %s"), error?error->message:_("Unknown error"));
      if (error)
	g_error_free (error);
      goto finalize;
    }

finalize:
  if (writer)
    xmlFreeTextWriter (writer);
  if (buf)
    xmlBufferFree (buf);
}

void
vinagre_bookmarks_add_entry (VinagreBookmarks      *book,
                             VinagreBookmarksEntry *entry,
                             VinagreBookmarksEntry *parent)
{
  /* I do not ref entry */
  if (parent)
    vinagre_bookmarks_entry_add_child (parent, entry);
  else
    book->priv->entries = g_slist_insert_sorted (book->priv->entries,
						 entry,
						 (GCompareFunc)vinagre_bookmarks_entry_compare);
  vinagre_bookmarks_save_to_file (book);
}

gboolean
vinagre_bookmarks_remove_entry (VinagreBookmarks      *book,
				VinagreBookmarksEntry *entry)
{
  GSList *l;

  g_return_val_if_fail (VINAGRE_IS_BOOKMARKS (book), FALSE);

  /* I do unref entry */
  if (g_slist_index (book->priv->entries, entry) > -1)
    {
      book->priv->entries = g_slist_remove (book->priv->entries, entry);
      g_object_unref (entry);
      vinagre_bookmarks_save_to_file (book);
      return TRUE;
    }

  for (l = book->priv->entries; l; l = l->next)
    {
      VinagreBookmarksEntry *e = (VinagreBookmarksEntry *) l->data;

      if (vinagre_bookmarks_entry_get_node (e) != VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER)
	continue;

      if (vinagre_bookmarks_entry_remove_child (e, entry))
	{
	  g_object_unref (entry);
	  vinagre_bookmarks_save_to_file (book);
	  return TRUE;
	}
    }

  return FALSE;
}

/* vim: set ts=8: */
