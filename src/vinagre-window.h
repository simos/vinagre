/*
 * vinagre-window.h
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
 * MERCHANWINDOWILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VINAGRE_WINDOW_H__
#define __VINAGRE_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_WINDOW              (vinagre_window_get_type())
#define VINAGRE_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_WINDOW, VinagreWindow))
#define VINAGRE_WINDOW_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_WINDOW, VinagreWindow const))
#define VINAGRE_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_WINDOW, VinagreWindowClass))
#define VINAGRE_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_WINDOW))
#define VINAGRE_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_WINDOW))
#define VINAGRE_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_WINDOW, VinagreWindowClass))

typedef struct _VinagreWindowPrivate VinagreWindowPrivate;
typedef struct _VinagreWindow        VinagreWindow;
typedef struct _VinagreWindowClass   VinagreWindowClass;

#include "vinagre-tab.h"
#include "vinagre-bookmarks.h"

struct _VinagreWindow 
{
  GtkWindow window;
  VinagreWindowPrivate *priv;
};

struct _VinagreWindowClass 
{
  GtkWindowClass parent_class;
};

GType 		 vinagre_window_get_type 		(void) G_GNUC_CONST;

VinagreWindow   *vinagre_window_new			(void);

VinagreTab	*vinagre_window_create_tab		(VinagreWindow         *window,
							 gboolean              jump_to);
							 
void		 vinagre_window_close_tab		(VinagreWindow         *window,
							 VinagreTab            *tab);
void		 vinagre_window_close_active_tab	(VinagreWindow         *window);

void		 vinagre_window_close_all_tabs		(VinagreWindow         *window);

void		 vinagre_window_close_tabs		(VinagreWindow         *window,
							 const GList           *tabs);
							 
VinagreTab	*vinagre_window_get_active_tab		(VinagreWindow         *window);

void		 vinagre_window_set_active_tab		(VinagreWindow         *window,
							 VinagreTab            *tab);

GtkWidget	*vinagre_window_get_statusbar		(VinagreWindow         *window);
GtkWidget	*vinagre_window_get_toolbar		(VinagreWindow         *window);
GtkWidget	*vinagre_window_get_menubar		(VinagreWindow         *window);
GtkWidget	*vinagre_window_get_fav_panel		(VinagreWindow         *window);
GtkWidget	*vinagre_window_get_notebook		(VinagreWindow	       *window);
GtkActionGroup	*vinagre_window_get_main_action		(VinagreWindow         *window);
GtkActionGroup	*vinagre_window_get_sensitive_action	(VinagreWindow         *window);
GtkActionGroup	*vinagre_window_get_connected_action	(VinagreWindow         *window);

void		vinagre_window_update_bookmarks_list_menu (VinagreWindow       *window);

GtkUIManager	*vinagre_window_get_ui_manager		(VinagreWindow         *window);

gboolean	vinagre_window_is_fullscreen		(VinagreWindow         *window);

void		vinagre_window_toggle_fullscreen	(VinagreWindow *window);
void		vinagre_window_set_title		(VinagreWindow *window);

void		vinagre_window_update_machine_menu_sensitivity (VinagreWindow	*window);

GList 		*vinagre_window_get_connections		(VinagreWindow *window);
VinagreTab	*vinagre_window_conn_exists		(VinagreWindow *window,
							 VinagreConnection *conn);

G_END_DECLS

#endif  /* __VINAGRE_WINDOW_H__  */
/* vim: set ts=8: */
