/*
 * vinagre-window.c
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

#include <time.h>
#include <sys/types.h>
#include <string.h>

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "vinagre-window.h"
#include "vinagre-notebook.h"
#include "vinagre-fav.h"
#include "vinagre-prefs.h"
#include "vinagre-utils.h"
#include "vinagre-bookmarks.h"
#include "vinagre-ui.h"
#include "vinagre-window-private.h"
#include "vinagre-bookmarks-entry.h"

#ifdef VINAGRE_ENABLE_AVAHI
#include "vinagre-mdns.h"
#endif

#define VINAGRE_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					 VINAGRE_TYPE_WINDOW,                    \
					 VinagreWindowPrivate))

G_DEFINE_TYPE(VinagreWindow, vinagre_window, GTK_TYPE_WINDOW)

static void
vinagre_window_dispose (GObject *object)
{
  VinagreWindow *window = VINAGRE_WINDOW (object);

  if (window->priv->fav_entry_selected)
    {
      g_object_unref (window->priv->fav_entry_selected);
      window->priv->fav_entry_selected = NULL;
    }

  if (window->priv->manager)
    {
      g_object_unref (window->priv->manager);
      window->priv->manager = NULL;
    }

  if (window->priv->signal_notebook != 0)
    {
      g_signal_handler_disconnect (window->priv->notebook,
				   window->priv->signal_notebook);
      window->priv->signal_notebook = 0;
    }

  if (window->priv->signal_clipboard != 0)
    {
      GtkClipboard  *cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);;

      g_signal_handler_disconnect (cb,
				   window->priv->signal_clipboard);
      window->priv->signal_clipboard = 0;
    }

  if (window->priv->update_recents_menu_ui_id != 0)
    {
      GtkRecentManager *recent_manager = gtk_recent_manager_get_default ();

      g_signal_handler_disconnect (recent_manager,
				   window->priv->update_recents_menu_ui_id);
      window->priv->update_recents_menu_ui_id = 0;
    }

  G_OBJECT_CLASS (vinagre_window_parent_class)->dispose (object);
}

static gboolean
vinagre_window_delete_event (GtkWidget   *widget,
			     GdkEventAny *event)
{
  VinagreWindow *window = VINAGRE_WINDOW (widget);

  vinagre_window_close_all_tabs (window);

  if (GTK_WIDGET_CLASS (vinagre_window_parent_class)->delete_event)
    return GTK_WIDGET_CLASS (vinagre_window_parent_class)->delete_event (widget, event);
  else
    return FALSE;
}

static void
vinagre_window_show_hide_controls (VinagreWindow *window)
{
  if (window->priv->fullscreen)
    {
      window->priv->fav_panel_visible = GTK_WIDGET_VISIBLE (window->priv->fav_panel);
      gtk_widget_hide (window->priv->fav_panel);

      window->priv->toolbar_visible = GTK_WIDGET_VISIBLE (window->priv->toolbar);
      gtk_widget_hide (window->priv->toolbar);

      gtk_widget_hide (window->priv->menubar);

      window->priv->statusbar_visible = GTK_WIDGET_VISIBLE (window->priv->statusbar);
      gtk_widget_hide (window->priv->statusbar);

      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (window->priv->notebook), FALSE);
      gtk_notebook_set_show_border (GTK_NOTEBOOK (window->priv->notebook), FALSE);
    }
  else
    {
      if (window->priv->fav_panel_visible)
        gtk_widget_show_all (window->priv->fav_panel);

      if (window->priv->toolbar_visible)
        gtk_widget_show_all (window->priv->toolbar);

      gtk_widget_show_all (window->priv->menubar);

      if (window->priv->statusbar_visible)
        gtk_widget_show_all (window->priv->statusbar);

      vinagre_notebook_show_hide_tabs (VINAGRE_NOTEBOOK (window->priv->notebook));
      gtk_notebook_set_show_border (GTK_NOTEBOOK (window->priv->notebook), TRUE);
    }
}

static gboolean
vinagre_window_state_event_cb (GtkWidget *widget,
			       GdkEventWindowState *event)
{
  VinagreWindow *window = VINAGRE_WINDOW (widget);

  window->priv->window_state = event->new_window_state;
  g_object_set (vinagre_prefs_get_default (),
		"window-state", window->priv->window_state,
		NULL);

  if ((event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) == 0)
    return FALSE;

  if (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN)
    window->priv->fullscreen = TRUE;
  else
    window->priv->fullscreen = FALSE;

  vinagre_window_show_hide_controls (window);

  if (GTK_WIDGET_CLASS (vinagre_window_parent_class)->window_state_event)
    return GTK_WIDGET_CLASS (vinagre_window_parent_class)->window_state_event (widget, event);
  else
    return FALSE;
}

static gboolean 
vinagre_window_configure_event (GtkWidget         *widget,
			        GdkEventConfigure *event)
{
  VinagreWindow *window = VINAGRE_WINDOW (widget);

  window->priv->width  = event->width;
  window->priv->height = event->height;

  g_object_set (vinagre_prefs_get_default (),
		"window-width", window->priv->width,
		"window-height", window->priv->height,
		NULL);

  return GTK_WIDGET_CLASS (vinagre_window_parent_class)->configure_event (widget, event);
}

static void
vinagre_window_class_init (VinagreWindowClass *klass)
{
  GObjectClass *object_class    = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS (klass);

  object_class->dispose  = vinagre_window_dispose;

  widget_class->window_state_event = vinagre_window_state_event_cb;
  widget_class->configure_event    = vinagre_window_configure_event;
  widget_class->delete_event       = vinagre_window_delete_event;

  g_type_class_add_private (object_class, sizeof(VinagreWindowPrivate));
}

static void
menu_item_select_cb (GtkItem       *proxy,
		     VinagreWindow *window)
{
  GtkAction *action;
  char *message;

  action = g_object_get_data (G_OBJECT (proxy),  "gtk-action");
  g_return_if_fail (action != NULL);

  g_object_get (G_OBJECT (action), "tooltip", &message, NULL);
  if (message)
    {
      gtk_statusbar_push (GTK_STATUSBAR (window->priv->statusbar),
			  window->priv->tip_message_cid, message);
      g_free (message);
    }
}

static void
menu_item_deselect_cb (GtkItem       *proxy,
                       VinagreWindow *window)
{
  gtk_statusbar_pop (GTK_STATUSBAR (window->priv->statusbar),
		     window->priv->tip_message_cid);
}

static void
connect_proxy_cb (GtkUIManager  *manager,
                  GtkAction     *action,
                  GtkWidget     *proxy,
                  VinagreWindow *window)
{
  if (GTK_IS_MENU_ITEM (proxy))
    {
       g_signal_connect (proxy, "select",
			 G_CALLBACK (menu_item_select_cb), window);
       g_signal_connect (proxy, "deselect",
			 G_CALLBACK (menu_item_deselect_cb), window);
    }
}

static void
disconnect_proxy_cb (GtkUIManager  *manager,
                     GtkAction     *action,
                     GtkWidget     *proxy,
                     VinagreWindow *window)
{
  if (GTK_IS_MENU_ITEM (proxy))
    {
      g_signal_handlers_disconnect_by_func
		(proxy, G_CALLBACK (menu_item_select_cb), window);
      g_signal_handlers_disconnect_by_func
		(proxy, G_CALLBACK (menu_item_deselect_cb), window);
    }
}

static void
activate_recent_cb (GtkRecentChooser *action, VinagreWindow *window)
{
  VinagreConnection *conn;
  gchar             *error;

  conn = vinagre_connection_new_from_string (gtk_recent_chooser_get_current_uri (action),
					     &error,
					     TRUE);
  if (conn)
    {
      vinagre_cmd_open_bookmark (window, conn);
      g_object_unref (conn);
    }
  else
    vinagre_utils_show_error (NULL, error ? error : _("Unknown error"), GTK_WINDOW (window));

  g_free (error);
}

static void
update_recent_connections (VinagreWindow *window)
{
  VinagreWindowPrivate *p = window->priv;

  g_return_if_fail (p->recent_action_group != NULL);

  if (p->recents_menu_ui_id != 0)
    gtk_ui_manager_remove_ui (p->manager, p->recents_menu_ui_id);

  p->recents_menu_ui_id = gtk_ui_manager_new_merge_id (p->manager);

  gtk_ui_manager_add_ui (p->manager,
			 p->recents_menu_ui_id,
			 "/MenuBar/MachineMenu/FileRecentsPlaceholder",
			 "recent_connections",
			 "recent_connections",
			 GTK_UI_MANAGER_MENUITEM,
			 FALSE);
}

static void
recent_manager_changed (GtkRecentManager *manager,
			VinagreWindow     *window)
{
  update_recent_connections (window);
}

static void
show_hide_accels (VinagreWindow *window)
{
  gboolean show_accels;

  g_object_get (vinagre_prefs_get_default (),
		"show-accels", &show_accels,
		NULL);
  g_object_set (gtk_settings_get_default (),
		"gtk-enable-accels", show_accels,
		"gtk-enable-mnemonics", show_accels,
		NULL);
}

static void
create_menu_bar_and_toolbar (VinagreWindow *window, 
			     GtkWidget     *main_box)
{
  GtkActionGroup   *action_group;
  GtkUIManager     *manager;
  GError           *error = NULL;
  GtkRecentManager *recent_manager;
  GtkRecentFilter  *filter;
  GtkAction        *action;

  manager = gtk_ui_manager_new ();
  window->priv->manager = manager;

  /* show tooltips in the statusbar */
  g_signal_connect (manager,
		    "connect-proxy",
		    G_CALLBACK (connect_proxy_cb),
		    window);
  g_signal_connect (manager,
		   "disconnect-proxy",
		    G_CALLBACK (disconnect_proxy_cb),
		    window);

  gtk_window_add_accel_group (GTK_WINDOW (window),
			      gtk_ui_manager_get_accel_group (manager));

  action_group = gtk_action_group_new ("VinagreWindowAlwaysSensitiveActions");
  gtk_action_group_set_translation_domain (action_group, NULL);
  gtk_action_group_add_actions (action_group,
				vinagre_always_sensitive_menu_entries,
				G_N_ELEMENTS (vinagre_always_sensitive_menu_entries),
				window);
  
  gtk_action_group_add_toggle_actions (action_group,
				       vinagre_always_sensitive_toggle_menu_entries,
				       G_N_ELEMENTS (vinagre_always_sensitive_toggle_menu_entries),
				       window);

  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);
  window->priv->always_sensitive_action_group = action_group;

  action = gtk_action_group_get_action (action_group, "MachineConnect");
  g_object_set (action, "is_important", TRUE, NULL);

  action_group = gtk_action_group_new ("VinagreWindowActions");
  gtk_action_group_set_translation_domain (action_group, NULL);
  gtk_action_group_add_actions (action_group,
				vinagre_menu_entries,
				G_N_ELEMENTS (vinagre_menu_entries),
				window);

  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);
  window->priv->action_group = action_group;

  action = gtk_action_group_get_action (action_group, "MachineClose");
  g_object_set (action, "is_important", TRUE, NULL);

  /* Machine connected actions */
  action_group = gtk_action_group_new ("VinagreWindowMachineConnectedActions");
  gtk_action_group_set_translation_domain (action_group, NULL);
  gtk_action_group_add_actions (action_group,
				vinagre_machine_connected_menu_entries,
				G_N_ELEMENTS (vinagre_machine_connected_menu_entries),
				window);
  gtk_action_group_add_toggle_actions  (action_group,
					vinagre_machine_connected_toggle_menu_entries,
					G_N_ELEMENTS (vinagre_machine_connected_toggle_menu_entries),
					window);

  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);
  window->priv->machine_connected_action_group = action_group;

  action = gtk_action_group_get_action (action_group, "ViewFullScreen");
  g_object_set (action, "is_important", TRUE, NULL);
  action = gtk_action_group_get_action (action_group, "MachineTakeScreenshot");
  g_object_set (action, "is_important", TRUE, NULL);
  action = gtk_action_group_get_action (action_group, "MachineSendCtrlAltDel");
  g_object_set (action, "is_important", TRUE, NULL);

  /* now load the UI definition */
  gtk_ui_manager_add_ui_from_file (manager, vinagre_utils_get_ui_xml_filename (), &error);
  if (error != NULL)
    {
      g_warning (_("Could not merge vinagre-ui.xml: %s"), error->message);
      g_error_free (error);
    }

  /* Bookmarks */
  action_group = gtk_action_group_new ("BookmarksActions");
  gtk_action_group_set_translation_domain (action_group, NULL);
  window->priv->bookmarks_list_action_group = action_group;
  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);

  window->priv->menubar = gtk_ui_manager_get_widget (manager, "/MenuBar");
  gtk_box_pack_start (GTK_BOX (main_box), 
		      window->priv->menubar, 
		      FALSE, 
		      FALSE, 
		      0);

  window->priv->toolbar = gtk_ui_manager_get_widget (manager, "/ToolBar");
  gtk_widget_hide (window->priv->toolbar);
  gtk_box_pack_start (GTK_BOX (main_box),
		      window->priv->toolbar,
		      FALSE,
		      FALSE,
		      0);

  /* Recent connections */
  window->priv->recent_action = gtk_recent_action_new ("recent_connections",
						     _("_Recent connections"),
						       NULL, NULL);
  g_object_set (G_OBJECT (window->priv->recent_action),
		"show-not-found", TRUE,
		"local-only", FALSE,
		"show-private", TRUE,
		"show-tips", TRUE,
		"sort-type", GTK_RECENT_SORT_MRU,
		NULL);
  gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (window->priv->recent_action), FALSE);
  g_signal_connect (window->priv->recent_action,
		    "item-activated",
		    G_CALLBACK (activate_recent_cb),
		    window);

  action_group = gtk_action_group_new ("VinagreRecentConnectionsActions");
  gtk_action_group_set_translation_domain (action_group, NULL);

  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);
  window->priv->recent_action_group = action_group;
  gtk_action_group_add_action (window->priv->recent_action_group,
			       window->priv->recent_action);

  filter = gtk_recent_filter_new ();
  gtk_recent_filter_add_group (filter, "vinagre");
  gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (window->priv->recent_action),
				 filter);

  update_recent_connections (window);

  recent_manager = gtk_recent_manager_get_default ();
  window->priv->update_recents_menu_ui_id = g_signal_connect (
		    recent_manager,
		    "changed",
		    G_CALLBACK (recent_manager_changed),
		    window);

  g_signal_connect_swapped (vinagre_prefs_get_default (),
			    "notify::show-accels",
			     G_CALLBACK (show_hide_accels),
			     window);
  show_hide_accels (window);
}

void
vinagre_window_update_machine_menu_sensitivity (VinagreWindow *window)
{
  gboolean active;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  active = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->priv->notebook)) > 0;
  gtk_action_group_set_sensitive (window->priv->action_group, active);

  active = (window->priv->active_tab) &&
	   (vinagre_tab_get_state (VINAGRE_TAB (window->priv->active_tab)) == VINAGRE_TAB_STATE_CONNECTED);
  gtk_action_group_set_sensitive (window->priv->machine_connected_action_group, active);
}

static void
fav_panel_size_allocate (GtkWidget     *widget,
			 GtkAllocation *allocation,
			 VinagreWindow *window)
{
  window->priv->side_panel_size = allocation->width;
  if (window->priv->side_panel_size > 0)
    g_object_set (vinagre_prefs_get_default (),
		  "side-panel-size", window->priv->side_panel_size,
		  NULL);
}

static void
fav_panel_activated (VinagreFav            *fav,
		     VinagreBookmarksEntry *entry,
		     VinagreWindow         *window)
{
  g_return_if_fail (vinagre_bookmarks_entry_get_node (entry) == VINAGRE_BOOKMARKS_ENTRY_NODE_CONN);

  vinagre_cmd_open_bookmark (window, vinagre_bookmarks_entry_get_conn (entry));
}

static void
fav_panel_selected (VinagreFav            *fav,
		    VinagreBookmarksEntry *entry,
		    VinagreWindow         *window)
{
  GtkAction *action1, *action2;

  action1 = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					 "BookmarksDel");
  action2 = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					 "BookmarksEdit");

  if (window->priv->fav_entry_selected)
    {
      g_object_unref (window->priv->fav_entry_selected);
      window->priv->fav_entry_selected = NULL;
    }

  if (entry)
    {
      window->priv->fav_entry_selected = g_object_ref (entry);
      gtk_action_set_sensitive (action1, TRUE);
      gtk_action_set_sensitive (action2, TRUE);
      /* TODO: Change the menuitem label */
    }
  else
    {
      /* TODO: Change the menuitem label */
      gtk_action_set_sensitive (action1, FALSE);
      gtk_action_set_sensitive (action2, FALSE);
    }
}

static void
vinagre_window_populate_bookmarks (VinagreWindow *window,
				   const gchar   *group,
				   GSList        *entries,
				   const gchar   *parent)
{
  static guint           i = 0;
  GSList                *l;
  VinagreBookmarksEntry *entry;
  gchar                 *action_name, *action_label, *path, *tooltip;
  GtkAction             *action;
  VinagreWindowPrivate  *p = window->priv;
  VinagreConnection     *conn;

  for (l = entries; l; l = l->next)
    {
      entry = VINAGRE_BOOKMARKS_ENTRY (l->data);
      switch (vinagre_bookmarks_entry_get_node (entry))
	{
	  case VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER:
	    action_label = vinagre_utils_escape_underscores (vinagre_bookmarks_entry_get_name (entry), -1);
	    action_name = g_strdup_printf ("BOOKMARK_FOLDER_ACTION_%d", ++i);
	    action = gtk_action_new (action_name,
				     action_label,
				     NULL,
				     NULL);
	    g_object_set (G_OBJECT (action), "icon-name", "folder", "hide-if-empty", FALSE, NULL);
	    gtk_action_group_add_action (p->bookmarks_list_action_group,
					 action);
	    g_object_unref (action);

	    path = g_strdup_printf ("/MenuBar/BookmarksMenu/%s%s", group, parent?parent:"");
	    gtk_ui_manager_add_ui (p->manager,
				   p->bookmarks_list_menu_ui_id,
				   path,
				   action_label, action_name,
				   GTK_UI_MANAGER_MENU,
				   FALSE);
	    g_free (path);
    	    g_free (action_name);

	    path = g_strdup_printf ("%s/%s", parent?parent:"", action_label);
	    g_free (action_label);

	    vinagre_window_populate_bookmarks (window, group, vinagre_bookmarks_entry_get_children (entry), path);
	    g_free (path);
	    break;

	  case VINAGRE_BOOKMARKS_ENTRY_NODE_CONN:
	    conn = vinagre_bookmarks_entry_get_conn (entry);

	    action_name = vinagre_connection_get_best_name (conn);
	    action_label = vinagre_utils_escape_underscores (action_name, -1);
	    g_free (action_name);

	    action_name = g_strdup_printf ("BOOKMARK_ITEM_ACTION_%d", ++i);

	    /* Translators: This is server:port, a statusbar tooltip when mouse is over a bookmark item on menu */
	    tooltip = g_strdup_printf (_("Open %s:%d"),
					vinagre_connection_get_host (conn),
					vinagre_connection_get_port (conn));
	    action = gtk_action_new (action_name,
				     action_label,
				     tooltip,
				     NULL);
	    g_object_set (G_OBJECT (action), "icon-name", "application-x-vnc", NULL);
	    g_object_set_data (G_OBJECT (action), "conn", conn);
	    gtk_action_group_add_action (p->bookmarks_list_action_group,
					 action);
	    g_signal_connect (action,
			     "activate",
			     G_CALLBACK (vinagre_cmd_bookmarks_open),
			     window);

	    path = g_strdup_printf ("/MenuBar/BookmarksMenu/%s%s", group, parent?parent:"");
	    gtk_ui_manager_add_ui (p->manager,
				   p->bookmarks_list_menu_ui_id,
				   path,
				   action_label, action_name,
				   GTK_UI_MANAGER_MENUITEM,
				   FALSE);

	    g_object_unref (action);
	    g_free (action_name);
	    g_free (action_label);
	    g_free (tooltip);
	    g_free (path);
	    break;

	  default:
	    g_assert_not_reached ();
	}
    }
}

void
vinagre_window_update_bookmarks_list_menu (VinagreWindow *window)
{
  VinagreWindowPrivate *p = window->priv;
  GList  *actions, *l;
  GSList *favs, *mdnss = NULL;

  g_return_if_fail (p->bookmarks_list_action_group != NULL);

  if (p->bookmarks_list_menu_ui_id != 0)
    gtk_ui_manager_remove_ui (p->manager, p->bookmarks_list_menu_ui_id);

  actions = gtk_action_group_list_actions (p->bookmarks_list_action_group);
  for (l = actions; l != NULL; l = l->next)
    {
      g_signal_handlers_disconnect_by_func (GTK_ACTION (l->data),
					    G_CALLBACK (vinagre_cmd_bookmarks_open),
					    window);
      gtk_action_group_remove_action (p->bookmarks_list_action_group,
				      GTK_ACTION (l->data));
    }
  g_list_free (actions);

  favs = vinagre_bookmarks_get_all (vinagre_bookmarks_get_default ());

#ifdef VINAGRE_ENABLE_AVAHI
  mdnss = vinagre_mdns_get_all (vinagre_mdns_get_default ());
#endif

  p->bookmarks_list_menu_ui_id = (g_slist_length (favs) > 0||g_slist_length (mdnss) > 0) ? gtk_ui_manager_new_merge_id (p->manager) : 0;
  vinagre_window_populate_bookmarks (window, "BookmarksList", favs, NULL);
  vinagre_window_populate_bookmarks (window, "AvahiList", mdnss, NULL);
}

static void
create_side_panel (VinagreWindow *window)
{
  window->priv->fav_panel = vinagre_fav_new (window);

  gtk_paned_pack1 (GTK_PANED (window->priv->hpaned), 
		   window->priv->fav_panel, 
		   FALSE, 
		   FALSE);

  g_object_get (vinagre_prefs_get_default (),
		"side-panel-size", &(window->priv->side_panel_size),
		NULL);
  gtk_paned_set_position (GTK_PANED (window->priv->hpaned), window->priv->side_panel_size);

  g_signal_connect (window->priv->fav_panel,
		    "size-allocate",
		    G_CALLBACK (fav_panel_size_allocate),
		    window);
  g_signal_connect (window->priv->fav_panel,
		    "fav-activated",
		    G_CALLBACK (fav_panel_activated),
		    window);
  g_signal_connect (window->priv->fav_panel,
		    "fav-selected",
		    G_CALLBACK (fav_panel_selected),
		    window);
}

static void
init_widgets_visibility (VinagreWindow *window)
{
  GdkWindowState state;
  gint w, h;
  GtkAction *action;
  gboolean visible;
  VinagrePrefs *prefs = vinagre_prefs_get_default ();

  /* Remove and Edit bookmarks starts disabled */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"BookmarksDel");
  gtk_action_set_sensitive (action, FALSE);

  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"BookmarksEdit");
  gtk_action_set_sensitive (action, FALSE);

  /* side panel visibility */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"ViewSidePanel");
  g_object_get (prefs, "side-panel-visible", &visible, NULL);
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) != visible)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);

  /* toolbar visibility */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"ViewToolbar");
  g_object_get (prefs, "toolbar-visible", &visible, NULL);
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) != visible)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);

  /* statusbar visibility */
  action = gtk_action_group_get_action (window->priv->always_sensitive_action_group,
					"ViewStatusbar");
  g_object_get (prefs, "statusbar-visible", &visible, NULL);
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) != visible)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);

  g_object_get (prefs,
		"window-state", &state,
		"window-width", &w,
		"window-height", &h,
		NULL);

  gtk_window_set_default_size (GTK_WINDOW (window), w, h);

  if ((state & GDK_WINDOW_STATE_MAXIMIZED) != 0)
    gtk_window_maximize (GTK_WINDOW (window));
  else
    gtk_window_unmaximize (GTK_WINDOW (window));
}

static void
create_statusbar (VinagreWindow *window, 
		  GtkWidget   *main_box)
{
  window->priv->statusbar = gtk_statusbar_new ();

  window->priv->generic_message_cid = gtk_statusbar_get_context_id
	(GTK_STATUSBAR (window->priv->statusbar), "generic_message");
  window->priv->tip_message_cid = gtk_statusbar_get_context_id
	(GTK_STATUSBAR (window->priv->statusbar), "tip_message");

  gtk_box_pack_end (GTK_BOX (main_box),
		    window->priv->statusbar,
		    FALSE, 
		    TRUE, 
		    0);
}

void
vinagre_window_set_title (VinagreWindow *window)
{
  gchar *title, *name, *grab;

  if (window->priv->active_tab == NULL)
    {
      gtk_window_set_title (GTK_WINDOW (window), g_get_application_name ());
      return;
    }

  if (vinagre_tab_is_pointer_grab (VINAGRE_TAB (window->priv->active_tab)))
    grab = g_strdup_printf (" (%s)", _("Press Ctrl+Alt to release the cursor"));
  else
    grab = g_strdup ("");

  name = vinagre_connection_get_best_name (vinagre_tab_get_conn (VINAGRE_TAB (window->priv->active_tab)));
  title = g_strdup_printf ("%s%s - %s",
			   name,
			   grab,
			   g_get_application_name ());
  gtk_window_set_title (GTK_WINDOW (window), title);
  g_free (title);
  g_free (name);
  g_free (grab);
}

static void
update_toggle_machine_items (VinagreWindow *window)
{
  GtkAction *action;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  if (window->priv->active_tab == NULL)
    {
      action = gtk_action_group_get_action (window->priv->machine_connected_action_group, "ViewScaling");
      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);

      action = gtk_action_group_get_action (window->priv->machine_connected_action_group, "ViewReadOnly");
      gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);

      return;
    }

  action = gtk_action_group_get_action (window->priv->machine_connected_action_group, "ViewScaling");
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
				vinagre_tab_get_scaling (VINAGRE_TAB (window->priv->active_tab)));

  action = gtk_action_group_get_action (window->priv->machine_connected_action_group, "ViewReadOnly");
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
				vinagre_tab_get_readonly (VINAGRE_TAB (window->priv->active_tab)));

}

static void
vinagre_window_page_removed (GtkNotebook   *notebook,
			     GtkWidget     *child,
			     guint         page_num,
			     VinagreWindow *window)
{
  GtkNotebook *nb;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  nb = GTK_NOTEBOOK (window->priv->notebook);

  window->priv->active_tab = gtk_notebook_get_nth_page (nb,
							gtk_notebook_get_current_page (nb));

  if (!window->priv->active_tab && window->priv->fullscreen)
    vinagre_window_toggle_fullscreen (window);

  vinagre_window_set_title (window);
  update_toggle_machine_items (window);
  vinagre_window_update_machine_menu_sensitivity (window);
}

static void
vinagre_window_page_added (GtkNotebook  *notebook,
			   GtkWidget     *child,
			   guint         page_num,
			   VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  window->priv->active_tab = child;

  vinagre_window_set_title (window);
  update_toggle_machine_items (window);
  vinagre_window_update_machine_menu_sensitivity (window);
}

static void 
vinagre_window_switch_page (GtkNotebook     *notebook, 
			    GtkNotebookPage *pg,
			    gint            page_num, 
			    VinagreWindow   *window)
{
  GtkWidget *tab;

  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  tab = gtk_notebook_get_nth_page (notebook, page_num);
  if (tab == window->priv->active_tab)
    return;

  window->priv->active_tab = tab;

  vinagre_window_set_title (window);
  update_toggle_machine_items (window);
  vinagre_window_update_machine_menu_sensitivity (window);
}

static void
create_notebook (VinagreWindow *window)
{
  window->priv->notebook = vinagre_notebook_new (window);

  gtk_paned_pack2 (GTK_PANED (window->priv->hpaned), 
  		   window->priv->notebook,
  		   TRUE, 
  		   FALSE);

  window->priv->signal_notebook =
   g_signal_connect_after (window->priv->notebook,
		          "page-removed",
		          G_CALLBACK (vinagre_window_page_removed),
		          window);

  g_signal_connect_after (window->priv->notebook,
		          "page-added",
		          G_CALLBACK (vinagre_window_page_added),
		          window);

  g_signal_connect (window->priv->notebook,
		    "switch_page",
		    G_CALLBACK (vinagre_window_switch_page),
		    window);

  gtk_widget_show (window->priv->notebook);
}

static void
vinagre_window_clipboard_cb (GtkClipboard *cb, GdkEvent *event, VinagreWindow *window)
{
  gchar *text;

  if (!window->priv->active_tab)
    return;

  if (gtk_clipboard_get_owner (cb) == G_OBJECT (window->priv->active_tab))
    return;

  text = gtk_clipboard_wait_for_text (cb);
  if (!text)
    return;

  vinagre_tab_paste_text (VINAGRE_TAB (window->priv->active_tab), text);
  g_free (text);
}

static void
vinagre_window_init_clipboard (VinagreWindow *window)
{
  GtkClipboard *cb;

  cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  window->priv->signal_clipboard = g_signal_connect (cb,
						     "owner-change",
						     G_CALLBACK (vinagre_window_clipboard_cb),
						     window);
}

static void
vinagre_window_init (VinagreWindow *window)
{
  GtkWidget *main_box;

  gtk_window_set_default_icon_name ("vinagre");

  window->priv = VINAGRE_WINDOW_GET_PRIVATE (window);
  window->priv->active_tab = NULL;
  window->priv->fav_entry_selected = NULL;
  window->priv->fullscreen = FALSE;
  window->priv->signal_notebook = 0;

  main_box = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), main_box);
  gtk_widget_show (main_box);

  /* Add menu bar and toolbar bar  */
  create_menu_bar_and_toolbar (window, main_box);

  /* Add status bar */
  create_statusbar (window, main_box);

  /* Add the main area */
  window->priv->hpaned = gtk_hpaned_new ();
  gtk_box_pack_start (GTK_BOX (main_box), 
  		      window->priv->hpaned, 
  		      TRUE, 
  		      TRUE, 
  		      0);

  /* setup notebook area */
  create_notebook (window);

  /* side panel */
  create_side_panel (window);

  gtk_widget_show (window->priv->hpaned);

  gtk_widget_grab_focus (window->priv->hpaned);

  init_widgets_visibility (window);
  vinagre_window_update_machine_menu_sensitivity (window);

  vinagre_window_update_bookmarks_list_menu (window);
  g_signal_connect_swapped (vinagre_bookmarks_get_default (),
                            "changed",
                            G_CALLBACK (vinagre_window_update_bookmarks_list_menu),
                            window);
#ifdef VINAGRE_ENABLE_AVAHI
  g_signal_connect_swapped (vinagre_mdns_get_default (),
                            "changed",
                            G_CALLBACK (vinagre_window_update_bookmarks_list_menu),
                            window);
#endif
  vinagre_window_init_clipboard (window);
}

GtkWidget *
vinagre_window_get_notebook (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->notebook;
}

void
vinagre_window_close_tab (VinagreWindow *window,
			  VinagreTab    *tab)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_TAB (tab));
	
  vinagre_notebook_remove_tab (VINAGRE_NOTEBOOK (window->priv->notebook),
			       tab);
}

void
vinagre_window_close_all_tabs (VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_notebook_remove_all_tabs (VINAGRE_NOTEBOOK (window->priv->notebook));
}


void
vinagre_window_set_active_tab (VinagreWindow *window,
			       VinagreTab    *tab)
{
  gint page_num;
	
  g_return_if_fail (VINAGRE_IS_WINDOW (window));
  g_return_if_fail (VINAGRE_IS_TAB (tab));
	
  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (window->priv->notebook),
				    GTK_WIDGET (tab));
  g_return_if_fail (page_num != -1);
	
  gtk_notebook_set_current_page (GTK_NOTEBOOK (window->priv->notebook),
				 page_num);
}

VinagreWindow *
vinagre_window_new ()
{
  return g_object_new (VINAGRE_TYPE_WINDOW,
		       "type",      GTK_WINDOW_TOPLEVEL,
		       "title",     g_get_application_name (),
		       NULL); 
}

gboolean
vinagre_window_is_fullscreen (VinagreWindow *window)
{
  return window->priv->fullscreen;
}

void
vinagre_window_toggle_fullscreen (VinagreWindow *window)
{
  if (window->priv->fullscreen)
    gtk_window_unfullscreen (GTK_WINDOW (window));
  else
    gtk_window_fullscreen (GTK_WINDOW (window));
}

GtkWidget *
vinagre_window_get_statusbar (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->statusbar;
}

GtkWidget *
vinagre_window_get_toolbar (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->toolbar;
}

GtkWidget *
vinagre_window_get_menubar (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->menubar;
}

GtkActionGroup *
vinagre_window_get_main_action (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->action_group;
}

GtkActionGroup *
vinagre_window_get_sensitive_action (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->always_sensitive_action_group;
}

GtkActionGroup *
vinagre_window_get_connected_action (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->machine_connected_action_group;
}

GtkWidget *
vinagre_window_get_fav_panel (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->fav_panel;
}

void
vinagre_window_close_active_tab	(VinagreWindow *window)
{
  g_return_if_fail (VINAGRE_IS_WINDOW (window));

  vinagre_window_close_tab (window,
			    VINAGRE_TAB (window->priv->active_tab));
}

VinagreTab *
vinagre_window_get_active_tab (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return VINAGRE_TAB (window->priv->active_tab);
}

GtkUIManager *
vinagre_window_get_ui_manager (VinagreWindow *window)
{
  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);

  return window->priv->manager;
}

static void
add_connection (VinagreTab *tab, GList **res)
{
  VinagreConnection *conn;
	
  conn = vinagre_tab_get_conn (tab);
	
  *res = g_list_prepend (*res, conn);
}

/* Returns a newly allocated list with all the connections in the window */
GList *
vinagre_window_get_connections (VinagreWindow *window)
{
  GList *res = NULL;

  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);
	
  gtk_container_foreach (GTK_CONTAINER (window->priv->notebook),
			 (GtkCallback)add_connection,
			  &res);
			       
  res = g_list_reverse (res);
	
  return res;
}

VinagreTab *
vinagre_window_conn_exists (VinagreWindow *window, VinagreConnection *conn)
{
  VinagreConnection *c;
  VinagreTab *tab = NULL;
  GList *l;

  g_return_val_if_fail (VINAGRE_IS_WINDOW (window), NULL);
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  l = vinagre_window_get_connections (window);

  while (l != NULL)
    {
      c = VINAGRE_CONNECTION (l->data);

      if (!strcmp (vinagre_connection_get_host (conn), vinagre_connection_get_host (c)) &&
	  vinagre_connection_get_port (conn) == vinagre_connection_get_port (c))
	{
	  tab = g_object_get_data (G_OBJECT (c), VINAGRE_TAB_KEY);
	  break;
	}
      l = l->next;
    }
  g_list_free (l);

  return tab;
}

/* vim: set ts=8: */
