/*
 * vinagre-commands.h
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
 
#ifndef __VINAGRE_COMMANDS_H__
#define __VINAGRE_COMMANDS_H__

#include <gtk/gtk.h>
#include "vinagre-window.h"
#include "vinagre-connection.h"

G_BEGIN_DECLS

void		vinagre_cmd_direct_connect	(VinagreConnection *conn,
						 VinagreWindow     *window);

void		vinagre_cmd_machine_connect	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_machine_open	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_machine_close	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_machine_take_screenshot (GtkAction     *action,
						     VinagreWindow *window);
void		vinagre_cmd_machine_send_ctrlaltdel (GtkAction     *action,
						     VinagreWindow *window);

void		vinagre_cmd_machine_close_all	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_machine_quit	(GtkAction     *action,
						 VinagreWindow *window);

void		vinagre_cmd_edit_preferences	(GtkAction     *action,
						 VinagreWindow *window);

void		vinagre_cmd_view_show_toolbar	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_view_show_statusbar	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_view_show_fav_panel	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_view_scaling	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_view_original_size	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_view_fullscreen	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_view_readonly	(GtkAction     *action,
						 VinagreWindow *window);

void		vinagre_cmd_open_bookmark	(VinagreWindow     *window,
						 VinagreConnection *conn);
void		vinagre_cmd_bookmarks_add	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_bookmarks_open	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_bookmarks_new_folder(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_bookmarks_edit	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_bookmarks_del	(GtkAction     *action,
						 VinagreWindow *window);

void		vinagre_cmd_help_contents	(GtkAction     *action,
						 VinagreWindow *window);
void		vinagre_cmd_help_about		(GtkAction     *action,
						 VinagreWindow *window);



G_END_DECLS

#endif /* __VINAGRE_COMMANDS_H__ */ 
/* vim: set ts=8: */
