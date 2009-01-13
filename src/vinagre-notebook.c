/*
 * vinagre-notebook.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008 - Jonh Wendell <wendell@bani.com.br>
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

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "vinagre-notebook.h"
#include "vinagre-utils.h"
#include "vinagre-prefs.h"

#define VINAGRE_NOTEBOOK_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), VINAGRE_TYPE_NOTEBOOK, VinagreNotebookPrivate))

struct _VinagreNotebookPrivate
{
  VinagreWindow *window;
};

G_DEFINE_TYPE(VinagreNotebook, vinagre_notebook, GTK_TYPE_NOTEBOOK)

static void
vinagre_notebook_class_init (VinagreNotebookClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (object_class, sizeof(VinagreNotebookPrivate));
}

GtkWidget *
vinagre_notebook_new (VinagreWindow *window)
{
  VinagreNotebook *nb = g_object_new (VINAGRE_TYPE_NOTEBOOK, NULL);

  nb->priv->window = window;
  return GTK_WIDGET (nb);
}

void
vinagre_notebook_show_hide_tabs (VinagreNotebook *nb)
{
  gboolean always;
  gint     n;

  n = gtk_notebook_get_n_pages (GTK_NOTEBOOK (nb));

  g_object_get (vinagre_prefs_get_default (),
		"always-show-tabs", &always,
		NULL);

  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (nb),
			      (n > 1) || (always));
}

static void
drag_data_get_handl (GtkWidget *widget,
		     GdkDragContext *context,
		     GtkSelectionData *selection_data,
		     guint target_type,
		     guint time,
		     VinagreTab *tab)
{
  gchar *uri, *name, *data;
  VinagreConnection *conn;

  g_assert (selection_data != NULL);

  switch (target_type)
    {
      case TARGET_VINAGRE:
	conn = vinagre_tab_get_conn (tab);
	uri = vinagre_connection_get_string_rep (conn, TRUE);
	name = vinagre_connection_get_best_name (conn);
	data = g_strdup_printf ("%s||%s", name, uri);

	gtk_selection_data_set (selection_data,
				selection_data->target,
				8,
				(guchar*) data,
				strlen (data));

	g_free (data);
	g_free (name);
	g_free (uri);
	break;

      case TARGET_STRING:
	/*FIXME: Create a .VNC file*/
	break;

      default:
	g_assert_not_reached ();
    }
}

static void
vinagre_notebook_init (VinagreNotebook *notebook)
{
  notebook->priv = VINAGRE_NOTEBOOK_GET_PRIVATE (notebook);

  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

  g_signal_connect (notebook,
		    "page-added",
		    G_CALLBACK (vinagre_notebook_show_hide_tabs),
		    NULL);
  g_signal_connect (notebook,
		    "page-removed",
		    G_CALLBACK (vinagre_notebook_show_hide_tabs),
		    NULL);
  g_signal_connect_swapped (vinagre_prefs_get_default (),
			    "notify::always-show-tabs",
			     G_CALLBACK (vinagre_notebook_show_hide_tabs),
			     notebook);
}

static void
close_button_clicked_cb (GtkWidget *widget, 
			 GtkWidget *tab)
{
  VinagreNotebook *notebook;

  notebook = VINAGRE_NOTEBOOK (gtk_widget_get_parent (tab));
  vinagre_notebook_remove_tab (notebook, VINAGRE_TAB (tab));
}

static void
tab_size_changed_cb (VinagreTab *tab, VinagreNotebook *nb)
{
  char *str;
  GtkWidget *label;

  label = GTK_WIDGET (g_object_get_data (G_OBJECT (tab), "label-ebox"));
  g_return_if_fail (label != NULL);

  str = vinagre_tab_get_tooltips (tab);
  gtk_widget_set_tooltip_markup (label, str);

  g_free (str);
}

static void
tab_disconnected_cb (VinagreTab *tab, VinagreNotebook *nb)
{
  gchar *message, *name;

  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (tab));
  message = g_strdup_printf (_("Connection to host <i>%s</i> was closed."),
			     name);
  vinagre_utils_show_error (_("Connection closed"), message, GTK_WINDOW (nb->priv->window));
  g_free (message);
  g_free (name);

  vinagre_notebook_remove_tab (nb, tab);
}

static GtkWidget *
build_tab_label (VinagreNotebook *nb, 
		 VinagreTab      *tab)
{
  GtkWidget *hbox, *label_hbox, *label_ebox;
  GtkWidget *label, *dummy_label;
  GtkWidget *close_button;
  GtkRcStyle *rcstyle;
  GtkWidget *image;
  GtkWidget *icon;
  gchar     *name;

  hbox = gtk_hbox_new (FALSE, 4);

  label_ebox = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (label_ebox), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), label_ebox, TRUE, TRUE, 0);
  gtk_widget_set_tooltip_text (label_ebox, _("Connecting..."));

  label_hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (label_ebox), label_hbox);

  /* setup close button */
  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button),
			 GTK_RELIEF_NONE);

  /* don't allow focus on the close button */
  gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);

  /* make it as small as possible */
  rcstyle = gtk_rc_style_new ();
  rcstyle->xthickness = rcstyle->ythickness = 0;
  gtk_widget_modify_style (close_button, rcstyle);
  g_object_unref (rcstyle);
  gint w, h;
  gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);
  gtk_widget_set_size_request (GTK_WIDGET (close_button), w+2, h+2);

  image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
					  GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (close_button), image);
  gtk_box_pack_start (GTK_BOX (hbox), close_button, FALSE, FALSE, 0);

  gtk_widget_set_tooltip_text (close_button, _("Close connection"));

  g_signal_connect (close_button,
		    "clicked",
		    G_CALLBACK (close_button_clicked_cb),
		    tab);

  /* setup site icon, empty by default */
  icon = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (label_hbox), icon, FALSE, FALSE, 0);
	
  /* setup label */
  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (tab));
  label = gtk_label_new (name);
  g_free (name);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 0, 0);
  gtk_box_pack_start (GTK_BOX (label_hbox), label, FALSE, FALSE, 0);

  dummy_label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (label_hbox), dummy_label, TRUE, TRUE, 0);
	
  gtk_widget_show (hbox);
  gtk_widget_show (label_ebox);
  gtk_widget_show (label_hbox);
  gtk_widget_show (label);
  gtk_widget_show (dummy_label);	
  gtk_widget_show (image);
  gtk_widget_show (close_button);
  gtk_widget_show (icon);
  
  g_object_set_data (G_OBJECT (hbox), "label", label);
  g_object_set_data (G_OBJECT (tab),  "label", label);
  g_object_set_data (G_OBJECT (hbox), "label-ebox", label_ebox);
  g_object_set_data (G_OBJECT (tab),  "label-ebox", label_ebox);
  g_object_set_data (G_OBJECT (hbox), "icon", icon);
  g_object_set_data (G_OBJECT (hbox), "close-button", close_button);
  g_object_set_data (G_OBJECT (tab),  "close-button", close_button);

  g_signal_connect (tab,
		    "notify::original-width",
		    G_CALLBACK (tab_size_changed_cb),
		    nb);

  g_signal_connect (tab,
		    "tab-disconnected",
		    G_CALLBACK (tab_disconnected_cb),
		    nb);

  gtk_drag_source_set ( GTK_WIDGET (hbox),
			GDK_BUTTON1_MASK,
			vinagre_target_list,
			vinagre_n_targets,
			GDK_ACTION_COPY );
  g_signal_connect (hbox,
		    "drag-data-get",
		    G_CALLBACK (drag_data_get_handl),
		    tab);

  return hbox;
}

void
vinagre_notebook_add_tab (VinagreNotebook *nb,
			  VinagreTab      *tab,
			  gint           position)
{
  GtkWidget *label;
  int pos;

  g_return_if_fail (VINAGRE_IS_NOTEBOOK (nb));
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  label = build_tab_label (nb, tab);

  pos = gtk_notebook_insert_page (GTK_NOTEBOOK (nb), 
				  GTK_WIDGET (tab),
				  label, 
				  position);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), pos);
  vinagre_tab_set_notebook (tab, nb);
}

static void
remove_tab (VinagreTab *tab,
	    VinagreNotebook *nb)
{
  vinagre_notebook_remove_tab (nb, tab);
}

void
vinagre_notebook_remove_tab (VinagreNotebook *nb,
			     VinagreTab      *tab)
{
  gint position;

  g_return_if_fail (VINAGRE_IS_NOTEBOOK (nb));
  g_return_if_fail (VINAGRE_IS_TAB (tab));

  position = gtk_notebook_page_num (GTK_NOTEBOOK (nb), GTK_WIDGET (tab));

  g_signal_handlers_disconnect_by_func (tab,
					G_CALLBACK (tab_disconnected_cb),
					nb);

  gtk_notebook_remove_page (GTK_NOTEBOOK (nb), position);
}

void
vinagre_notebook_remove_all_tabs (VinagreNotebook *nb)
{	
  g_return_if_fail (VINAGRE_IS_NOTEBOOK (nb));
	
  gtk_container_foreach (GTK_CONTAINER (nb),
			(GtkCallback) remove_tab,
			 nb);
}
/* vim: set ts=8: */
