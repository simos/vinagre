/*
 * vinagre-connect.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007 - Jonh Wendell <wendell@bani.com.br>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <string.h>

#ifdef VINAGRE_ENABLE_AVAHI
#include <avahi-ui/avahi-ui.h>
#endif

#include "vinagre-connect.h"
#include "vinagre-utils.h"
#include "vinagre-bookmarks.h"
#include "vinagre-prefs.h"

typedef struct {
  GladeXML  *xml;
  GtkWidget *dialog;
  GtkWidget *host_entry;
  GtkWidget *find_button;
  GtkWidget *fullscreen_check;
  GtkWidget *scaling_check;
  GtkWidget *viewonly_check;
} VinagreConnectDialog;

enum {
  COLUMN_TEXT,
  N_COLUMNS
};

static gchar*
history_filename () {
  return g_build_filename (g_get_user_data_dir (),
			   "vinagre",
			   "history",
			   NULL);
}

static GPtrArray *
saved_history (void)
{
  gchar *filename, *file_contents = NULL;
  gchar **history_from_file = NULL, **list;
  gboolean success;
  gint len;
  GPtrArray *array;

  array = g_ptr_array_new ();

  filename = history_filename ();
  success = g_file_get_contents (filename,
				 &file_contents,
				 NULL,
				 NULL);

  if (success)
    {
      history_from_file = g_strsplit (file_contents, "\n", 0);
      len = g_strv_length (history_from_file);
      if (strlen (history_from_file[len-1]) == 0)
	{
	  g_free (history_from_file[len-1]);
	  history_from_file[len-1] = NULL;
	}

      list = history_from_file;

      while (*list != NULL)
	g_ptr_array_add (array, *list++);
    }

  g_free (filename);
  g_free (file_contents);
  return array;
}

static void
setup_combo (GtkWidget *combo)
{
  GtkListStore *store;
  GtkTreeIter   iter;
  GtkEntryCompletion *completion;
  GPtrArray    *history;
  gint          i, size;

  store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);

  history = saved_history ();

  g_object_get (vinagre_prefs_get_default (), "history-size", &size, NULL);
  if (size <= 0)
    size = G_MAXINT;

  for (i=history->len-1; i>=0 && i>=(gint)history->len-size; i--)
   {
      GtkTreeIter iter;
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, COLUMN_TEXT, g_ptr_array_index (history, i), -1);
    }
  g_ptr_array_free (history, TRUE);

  gtk_combo_box_set_model (GTK_COMBO_BOX (combo),
			   GTK_TREE_MODEL (store));
  gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY (combo),
				       0);

  completion = gtk_entry_completion_new ();
  gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (store));
  gtk_entry_completion_set_text_column (completion, 0);
  gtk_entry_completion_set_inline_completion (completion, TRUE);
  gtk_entry_set_completion (GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo))), completion);
  g_object_unref (completion);
}

void
save_history (GtkWidget *combo) {
  gchar *host;
  GPtrArray *history;
  guint i;
  gchar *filename, *path;
  GString *content;
  GError *error = NULL;

  host = gtk_combo_box_get_active_text (GTK_COMBO_BOX (combo));

  history = saved_history ();
  for (i=0; i<history->len; i++)
    if (!g_strcmp0 (g_ptr_array_index (history, i), host))
      {
	g_ptr_array_remove_index (history, i);
	break;
      }

  g_ptr_array_add (history, host);
  content = g_string_new (NULL);

  for (i=0; i<history->len; i++)
    g_string_append_printf (content, "%s\n", (char *) g_ptr_array_index (history, i));

  filename = history_filename ();
  path = g_path_get_dirname (filename);
  g_mkdir_with_parents (path, 0755);

  g_file_set_contents (filename,
		       content->str,
		       -1,
		       &error);

  g_free (filename);
  g_free (path);
  g_ptr_array_free (history, TRUE);
  g_string_free (content, TRUE);

  if (error) {
    g_warning (_("Error while saving history file: %s"), error->message);
    g_error_free (error);
  }
}

#ifdef VINAGRE_ENABLE_AVAHI
static void
vinagre_connect_find_button_cb (GtkButton            *button,
				VinagreConnectDialog *dialog)
{
  GtkWidget *d;

  d = aui_service_dialog_new (_("Choose a VNC Server"),
				GTK_WINDOW(dialog->dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				NULL);
  gtk_window_set_transient_for (GTK_WINDOW(d), GTK_WINDOW(dialog->dialog));
  aui_service_dialog_set_resolve_service (AUI_SERVICE_DIALOG(d), TRUE);
  aui_service_dialog_set_resolve_host_name (AUI_SERVICE_DIALOG(d), TRUE);
  aui_service_dialog_set_browse_service_types (AUI_SERVICE_DIALOG(d),
					       "_rfb._tcp",
					       NULL);

  if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT)
    {
      gchar *tmp;

      tmp = g_strdup_printf ("%s::%d",
			     aui_service_dialog_get_host_name(AUI_SERVICE_DIALOG(d)),
			     aui_service_dialog_get_port(AUI_SERVICE_DIALOG(d)));

      gtk_entry_set_text (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (dialog->host_entry))),
			  tmp);

      g_free (tmp);
    }

  gtk_widget_destroy (d);
}
#endif

VinagreConnection *vinagre_connect (VinagreWindow *window)
{
  VinagreConnection    *conn = NULL;
  gint                  result;
  VinagreConnectDialog  dialog;

  dialog.xml = glade_xml_new (vinagre_utils_get_glade_filename (), NULL, NULL);
  dialog.dialog = glade_xml_get_widget (dialog.xml, "connect_dialog");
  gtk_window_set_transient_for (GTK_WINDOW (dialog.dialog), GTK_WINDOW (window));

  dialog.host_entry  = glade_xml_get_widget (dialog.xml, "host_entry");
  dialog.find_button = glade_xml_get_widget (dialog.xml, "find_button");
  dialog.fullscreen_check = glade_xml_get_widget (dialog.xml, "fullscreen_check");
  dialog.viewonly_check = glade_xml_get_widget (dialog.xml, "viewonly_check");
  dialog.scaling_check = glade_xml_get_widget (dialog.xml, "scaling_check");

  setup_combo (dialog.host_entry);

#ifdef VINAGRE_ENABLE_AVAHI
  g_signal_connect (dialog.find_button,
		    "clicked",
		    G_CALLBACK (vinagre_connect_find_button_cb),
		    &dialog);
#else
  gtk_widget_hide (dialog.find_button);
  gtk_widget_set_no_show_all (dialog.find_button, TRUE);
#endif

  gtk_widget_show_all (dialog.dialog);
  result = gtk_dialog_run (GTK_DIALOG (dialog.dialog));

  if (result == GTK_RESPONSE_OK)
    {
      gchar *host = NULL, *error_msg = NULL;

      host = gtk_combo_box_get_active_text (GTK_COMBO_BOX (dialog.host_entry));
      gtk_widget_hide (GTK_WIDGET (dialog.dialog));

      if (!host || !g_strcmp0 (host, ""))
	goto fail;

      save_history (dialog.host_entry);

      conn = vinagre_connection_new_from_string (host, &error_msg, TRUE);
      if (conn)
	{
	  g_object_set (conn,
			"scaling", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog.scaling_check)),
			"view-only", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog.viewonly_check)),
			"fullscreen", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog.fullscreen_check)),
			NULL);
	}
      else
	{
	  vinagre_utils_show_error (NULL, error_msg ? error_msg : _("Unknown error"),
				    GTK_WINDOW (window));
	}
fail:
      g_free (host);
      g_free (error_msg);
    }

  gtk_widget_destroy (dialog.dialog);
  g_object_unref (dialog.xml);
  return conn;
}
/* vim: set ts=8: */
