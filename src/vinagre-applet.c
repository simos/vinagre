/*
 * vinagre-applet.c
 * This file is part of vinagre
 *
 * Copyright (C) 2008 - Jonh Wendell <wendell@bani.com.br>
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

#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <panel-applet.h>
#include "vinagre-bookmarks.h"
#include "vinagre-bookmarks-entry.h"
#include "vinagre-utils.h"
#include "vinagre-connection.h"
#include "vinagre-commands.h"
#include <config.h>

#ifdef VINAGRE_ENABLE_AVAHI
#include "vinagre-mdns.h"
#endif

static void
menu_position (GtkMenu    *menu,
	       gint       *x,
	       gint       *y,
	       gboolean   *push_in,
	       GtkWidget  *applet)
{
  int applet_height, applet_width;
  GtkRequisition requisition;

  gdk_window_get_origin (applet->window, x, y);
  gdk_drawable_get_size (applet->window, &applet_width, &applet_height);
  gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

  switch (panel_applet_get_orient (PANEL_APPLET (applet)))
    {
      case PANEL_APPLET_ORIENT_DOWN:
	*y += applet_height + 1;
	break;

      case PANEL_APPLET_ORIENT_UP:
	*y = MAX (*y - requisition.height - 2, 2*applet_height);
	break;

      case PANEL_APPLET_ORIENT_LEFT:
	*x = MAX (*x - requisition.width - 2, 2*applet_width);
	break;

      case PANEL_APPLET_ORIENT_RIGHT:
	*x += applet_height + 1;
	break;

      default:
	g_assert_not_reached ();
    }

  *push_in = TRUE;
}

static void
open_connection_cb (GtkMenuItem *item,
		    gpointer    *user_data)
{
  GError *err = NULL;
  gchar **argv;
  VinagreConnection *conn;

  conn = VINAGRE_CONNECTION (g_object_get_data (G_OBJECT (item), "conn"));

  argv = g_new0 (gchar *, 3);
  argv[0] = g_strdup ("vinagre");
  argv[1] = g_strdup (vinagre_connection_get_string_rep (conn, TRUE));
  argv[2] = NULL;

  if (!gdk_spawn_on_screen (gtk_widget_get_screen (GTK_WIDGET (item)),
			    NULL,
			    argv,
			    NULL,
			    G_SPAWN_SEARCH_PATH,
			    NULL,
			    NULL,
			    NULL,
			    &err))
    {
      vinagre_utils_show_error (_("Could not run vinagre:"), err->message, NULL);
      g_error_free (err);
    }

  g_strfreev (argv);
}

static void
fill_recursive_menu (GSList *entries, GtkWidget *menu)
{
  GSList    *l;
  GtkWidget *item, *image, *child;

  for (l = entries; l; l = l->next)
    {
      VinagreBookmarksEntry *entry = VINAGRE_BOOKMARKS_ENTRY (l->data);
      VinagreConnection     *conn;

      switch (vinagre_bookmarks_entry_get_node (entry))
	{
	  case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	    image = gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_MENU);
	    item = gtk_image_menu_item_new_with_label (vinagre_bookmarks_entry_get_name (entry));
	    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
					   image);
	    gtk_menu_shell_append (GTK_MENU_SHELL (menu),
				   item);

	    child = gtk_menu_new ();
	    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), child);

	    fill_recursive_menu (vinagre_bookmarks_entry_get_children (entry),
				 child);
	    break;

	  case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
	    conn = vinagre_bookmarks_entry_get_conn (entry);
	    image = gtk_image_new_from_icon_name ("application-x-vnc", GTK_ICON_SIZE_MENU);
	    item = gtk_image_menu_item_new_with_label (vinagre_connection_get_name (conn));
	    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
					   image);

	    gtk_menu_shell_append (GTK_MENU_SHELL (menu),
				   item);
	    g_object_set_data (G_OBJECT (item), "conn", conn);
	    g_signal_connect (item, "activate", G_CALLBACK (open_connection_cb), NULL);
	    break;

	  default:
	    g_assert_not_reached ();
	}
    }

}

static void
fill_menu (GSList *all, GtkWidget *menu)
{
  GtkWidget *item, *image;

  if (g_slist_length (all) == 0)
    return;

  /* Separator */
  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  fill_recursive_menu (all, menu);
}

static void
open_vinagre_cb (GtkMenuItem *item,
		 gpointer    *user_data)
{
  GError *err = NULL;

  if (!g_spawn_command_line_async ("vinagre", &err))
    {
      vinagre_utils_show_error (_("Could not run vinagre:"), err->message, NULL);
      g_error_free (err);
    }
}

static gboolean
click_cb (GtkWidget      *applet,
	  GdkEventButton *event,
	  gpointer        user_data)
{
  GtkWidget *menu, *item, *image;
  GSList *all;

  if ((event->type != GDK_BUTTON_PRESS) || (event->button != 1))
    return FALSE;

  menu = gtk_menu_new ();

  /* Open, first item */
  image = gtk_image_new_from_icon_name ("vinagre", GTK_ICON_SIZE_MENU);
  item = gtk_image_menu_item_new_with_label (_("Open Remote Desktop Viewer"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
				 image);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  g_signal_connect (item, "activate", G_CALLBACK (open_vinagre_cb), NULL);

  all = vinagre_bookmarks_get_all (vinagre_bookmarks_get_default ());
  fill_menu (all, menu);

#ifdef VINAGRE_ENABLE_AVAHI
  all = vinagre_mdns_get_all (vinagre_mdns_get_default ());
  fill_menu (all, menu);
#endif

  gtk_widget_show_all (menu);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, (GtkMenuPositionFunc) menu_position, applet, 
		  event->button, event->time);
  return TRUE;
}

static void
help_cb (BonoboUIComponent *ui_container,
	 gpointer           data,
	 const gchar       *cname)
{
  vinagre_utils_help_contents (NULL);
}

static void
about_cb (BonoboUIComponent *ui_container,
	 gpointer           data,
	 const gchar       *cname)
{
  vinagre_utils_help_about (NULL);
}

static gboolean
vinagre_applet_fill (PanelApplet *applet,
		     const gchar *iid,
		     gpointer     data)
{
  GtkWidget *image, *button;
  gchar *tmp;
  static const BonoboUIVerb menu_verbs[] = {
    BONOBO_UI_VERB ("VinagreHelp", help_cb),
    BONOBO_UI_VERB ("VinagreAbout", about_cb),
    BONOBO_UI_VERB_END
  };

#ifdef VINAGRE_ENABLE_AVAHI
  VinagreMdns *mdns;
#endif

  if (strcmp (iid, "OAFIID:GNOME_VinagreApplet") != 0)
    return FALSE;

  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
  gtk_window_set_default_icon_name ("vinagre");
  g_set_application_name (_("Remote Desktop Viewer"));

  image = gtk_image_new_from_icon_name ("vinagre", GTK_ICON_SIZE_LARGE_TOOLBAR);
  g_signal_connect (applet, "button-press-event", G_CALLBACK (click_cb), NULL);

  tmp = g_strdup_printf ("%s\n%s",
			_("Remote Desktop Viewer"),
			_("Access your bookmarks"));
  gtk_widget_set_tooltip_text (GTK_WIDGET (applet), tmp);
  g_free (tmp);

  panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
  panel_applet_setup_menu_from_file (applet, NULL,
				     DATADIR "/vinagre/GNOME_VinagreApplet.xml",
				     NULL, menu_verbs, NULL);

  gtk_container_add (GTK_CONTAINER (applet), image);
  gtk_widget_show_all (GTK_WIDGET (applet));

#ifdef VINAGRE_ENABLE_AVAHI
  mdns = vinagre_mdns_get_default ();
#endif

  return TRUE;
}


PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_VinagreApplet_Factory",
                             PANEL_TYPE_APPLET,
                             "VinagreApplet",
                             "0",
                             vinagre_applet_fill,
                             NULL);

