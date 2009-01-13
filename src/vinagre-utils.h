/*
 * vinagre-utils.h
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

#ifndef __VINAGRE_UTILS_H__
#define __VINAGRE_UTILS_H__

#include <gtk/gtk.h>
#include <glib.h>

/* useful macro */
#define GBOOLEAN_TO_POINTER(i) ((gpointer) ((i) ? 2 : 1))
#define GPOINTER_TO_BOOLEAN(i) ((gboolean) ((((gint)(i)) == 2) ? TRUE : FALSE))
#define IS_VALID_BOOLEAN(v) (((v == TRUE) || (v == FALSE)) ? TRUE : FALSE)

enum {
  TARGET_VINAGRE,
  TARGET_STRING
};

static GtkTargetEntry vinagre_target_list[] = {
  { "VINAGRE", 0, TARGET_VINAGRE },
  { "text/plain", 0, TARGET_STRING }
};
static guint vinagre_n_targets = G_N_ELEMENTS (vinagre_target_list);

enum { VINAGRE_ALL_WORKSPACES = 0xffffffff };

GtkWidget	*vinagre_utils_create_small_close_button (void);

void		vinagre_utils_show_error		(const gchar *title,
							 const gchar *message,
							 GtkWindow *parent);

void		vinagre_utils_show_many_errors		(const gchar *title,
							 GSList *items,
							 GtkWindow *parent);

void		vinagre_utils_toggle_widget_visible	(GtkWidget *widget);

const gchar	*vinagre_utils_get_glade_filename	(void);
const gchar	*vinagre_utils_get_ui_xml_filename	(void);

gchar		*vinagre_utils_escape_underscores	(const gchar *text,
							 gssize      length);

void		vinagre_utils_handle_debug		(void);

guint		vinagre_utils_get_current_workspace	(GdkScreen *screen);
guint		vinagre_utils_get_window_workspace	(GtkWindow *gtkwindow);
void		vinagre_utils_get_current_viewport	(GdkScreen    *screen,
							 gint         *x,
							 gint         *y);

void		vinagre_utils_help_contents		(GtkWindow *window);
void		vinagre_utils_help_about		(GtkWindow *window);
#endif  /* __VINAGRE_UTILS_H__  */
/* vim: set ts=8: */
