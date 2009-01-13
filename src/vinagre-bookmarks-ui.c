/*
 * vinagre-bookmarks-ui.c
 * This file is part of vinagre
 *
 * Copyright (C) 2008  Jonh Wendell <wendell@bani.com.br>
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

#include <glade/glade.h>
#include <string.h>
#include <glib/gi18n.h>

#include "vinagre-bookmarks-ui.h"
#include "vinagre-utils.h"
#include "vinagre-bookmarks-tree.h"

static void
show_dialog_folder (VinagreBookmarks      *book,
		    GtkWindow             *window,
		    VinagreBookmarksEntry *entry,
		    gboolean               is_add)
{
  GladeXML    *xml;
  GtkWidget   *dialog, *box, *tree, *name_entry;
  const gchar *name;

  xml = glade_xml_new (vinagre_utils_get_glade_filename (),
		       "bookmarks_add_edit_folder_dialog",
		       NULL);
  dialog     = glade_xml_get_widget (xml, "bookmarks_add_edit_folder_dialog");
  name_entry = glade_xml_get_widget (xml, "edit_bookmark_folder_name_entry");
  box        = glade_xml_get_widget (xml, "folder_box1");

  gtk_window_set_transient_for (GTK_WINDOW (dialog), window);
  gtk_entry_set_text (GTK_ENTRY (name_entry), vinagre_bookmarks_entry_get_name (entry));
  gtk_editable_set_position (GTK_EDITABLE (name_entry), -1);

  tree = vinagre_bookmarks_tree_new ();
  vinagre_bookmarks_tree_select_entry  (VINAGRE_BOOKMARKS_TREE (tree),
					vinagre_bookmarks_entry_get_parent (entry));
  gtk_box_pack_end (GTK_BOX (box), tree, TRUE, TRUE, 0);

  gtk_widget_show_all (dialog);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
    {
      if (is_add)
        g_object_unref (entry);
      goto finalize;
    }

  name = gtk_entry_get_text (GTK_ENTRY (name_entry));
  if (strlen (name) < 1)
    {
      vinagre_utils_show_error (NULL, _("Invalid name for this folder"), window);
      if (is_add)
        g_object_unref (entry);
      goto finalize;
    }

  vinagre_bookmarks_entry_set_name (entry, name);
  if (!is_add)
    {
      g_object_ref (entry);
      vinagre_bookmarks_remove_entry (book, entry);
    }
  vinagre_bookmarks_add_entry ( book,
				entry,
				vinagre_bookmarks_tree_get_selected_entry (VINAGRE_BOOKMARKS_TREE (tree)));

finalize:
  gtk_widget_destroy (GTK_WIDGET (dialog));
  g_object_unref (G_OBJECT (xml));
}

static void
show_dialog_conn (VinagreBookmarks      *book,
		  GtkWindow             *window,
		  VinagreBookmarksEntry *entry,
		  gboolean               is_add)
{
  gchar             *str, *host, *error_str;
  gint               port;
  GladeXML          *xml;
  GtkWidget         *dialog, *host_entry, *name_entry;
  GtkWidget         *fs_check, *sc_check, *vo_check;
  GtkWidget         *box, *tree;
  VinagreConnection *conn;
  const gchar       *name;

  xml = glade_xml_new (vinagre_utils_get_glade_filename (),
		       "bookmarks_add_edit_conn_dialog",
		       NULL);
  dialog     = glade_xml_get_widget (xml, "bookmarks_add_edit_conn_dialog");
  name_entry = glade_xml_get_widget (xml, "edit_bookmark_name_entry");
  host_entry = glade_xml_get_widget (xml, "edit_bookmark_host_entry");
  fs_check   = glade_xml_get_widget (xml, "edit_fullscreen_check");
  sc_check   = glade_xml_get_widget (xml, "edit_scaling_check");
  vo_check   = glade_xml_get_widget (xml, "edit_viewonly_check");
  box        = glade_xml_get_widget (xml, "folder_box");

  gtk_window_set_transient_for (GTK_WINDOW (dialog), window);
  conn = vinagre_bookmarks_entry_get_conn (entry);

  str = vinagre_connection_get_best_name (conn);
  gtk_entry_set_text (GTK_ENTRY (name_entry), str);
  gtk_editable_set_position (GTK_EDITABLE (name_entry), -1);
  g_free (str);

  str = vinagre_connection_get_string_rep (conn, FALSE);
  gtk_entry_set_text (GTK_ENTRY (host_entry), str);
  g_free (str);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fs_check),
				vinagre_connection_get_fullscreen (conn));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sc_check),
				vinagre_connection_get_scaling (conn));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (vo_check),
				vinagre_connection_get_view_only (conn));

  tree = vinagre_bookmarks_tree_new ();
  vinagre_bookmarks_tree_select_entry  (VINAGRE_BOOKMARKS_TREE (tree),
					vinagre_bookmarks_entry_get_parent (entry));
  gtk_box_pack_end (GTK_BOX (box), tree, TRUE, TRUE, 0);

  gtk_widget_show_all (dialog);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
    goto finalize;

  if (!vinagre_connection_split_string (gtk_entry_get_text (GTK_ENTRY (host_entry)),
					&host,
					&port,
					&error_str))
    {
      vinagre_utils_show_error (NULL, error_str, window);
      g_free (error_str);
      goto finalize;
    }

  vinagre_connection_set_host (conn, host);
  vinagre_connection_set_port (conn, port);
  vinagre_connection_set_view_only  (conn,
				     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (vo_check)));
  vinagre_connection_set_scaling    (conn,
				     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sc_check)));
  vinagre_connection_set_fullscreen (conn,
				     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fs_check)));

  name = gtk_entry_get_text (GTK_ENTRY (name_entry));
  if (strlen (name) < 1)
    if (strlen (vinagre_connection_get_name (conn)) < 1)
      name = vinagre_connection_get_host (conn);
    else
      name = vinagre_connection_get_name (conn);
  vinagre_connection_set_name (conn, name);

  if (!is_add)
    {
      g_object_ref (entry);
      vinagre_bookmarks_remove_entry (book, entry);
    }
  vinagre_bookmarks_add_entry ( book,
				entry,
				vinagre_bookmarks_tree_get_selected_entry (VINAGRE_BOOKMARKS_TREE (tree)));

finalize:
  gtk_widget_destroy (GTK_WIDGET (dialog));
  g_object_unref (G_OBJECT (xml));
}

void
vinagre_bookmarks_add (VinagreBookmarks  *book,
                       VinagreConnection *conn,
		       GtkWindow         *window)
{
  VinagreBookmarksEntry *entry;

  g_return_if_fail (VINAGRE_IS_BOOKMARKS (book));
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  entry = vinagre_bookmarks_entry_new_conn (conn);
  show_dialog_conn (book, window, entry, TRUE);
}

void
vinagre_bookmarks_edit (VinagreBookmarks      *book,
                        VinagreBookmarksEntry *entry,
		        GtkWindow             *window)
{
  g_return_if_fail (VINAGRE_IS_BOOKMARKS (book));
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry));

  g_object_ref (entry);

  switch (vinagre_bookmarks_entry_get_node (entry))
    {
      case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	show_dialog_folder (book, window, entry, FALSE);
	break;

      case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
        show_dialog_conn (book, window, entry, FALSE);
        break;

      default:
	g_assert_not_reached ();
    }

  g_object_unref (entry);
}

void
vinagre_bookmarks_del (VinagreBookmarks      *book,
                       VinagreBookmarksEntry *entry,
		       GtkWindow             *window)
{
  GtkWidget *dialog;
  gchar     *name, *title, *msg1, *msg2;
  GError    *error = NULL;
  GSList    *parent;

  g_return_if_fail (VINAGRE_IS_BOOKMARKS (book));
  g_return_if_fail (VINAGRE_IS_BOOKMARKS_ENTRY (entry));

  msg1 = g_strdup (_("Are you sure you want to remove <i>%s</i> from bookmarks?"));

  if (vinagre_bookmarks_entry_get_node (entry) == VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER)
    {
      name = g_strdup (vinagre_bookmarks_entry_get_name (entry));
      title = g_strdup (_("Remove Folder?"));
      msg2 = g_strdup_printf ("%s\n\n%s", msg1, _("Notice that all its subfolders and items will be removed as well."));
    }
  else
    {
      name = vinagre_connection_get_best_name (vinagre_bookmarks_entry_get_conn (entry));
      title = g_strdup (_("Remove Item?"));
      msg2 = g_strdup (msg1);
    }

  dialog = gtk_message_dialog_new (window,
				   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_QUESTION,
				   GTK_BUTTONS_OK_CANCEL,
				   "%s",
				   title);

  gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (dialog),
					      msg2,
					      name);
 
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    if (!vinagre_bookmarks_remove_entry (book, entry))
      g_warning (_("Error removing bookmark: Entry not found"));

  gtk_widget_destroy (dialog);
  g_free (name);
  g_free (title);
  g_free (msg1);
  g_free (msg2);
}

void
vinagre_bookmarks_new_folder (VinagreBookmarks *book,
			      GtkWindow        *window)
{
  VinagreBookmarksEntry *entry;

  g_return_if_fail (VINAGRE_IS_BOOKMARKS (book));

  entry = vinagre_bookmarks_entry_new_folder (_("New Folder"));
  show_dialog_folder (book, window, entry, TRUE);
}

/* vim: set ts=8: */
