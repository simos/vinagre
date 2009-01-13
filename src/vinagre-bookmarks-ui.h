/*
 * vinagre-bookmarks-ui.h
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

#ifndef __VINAGRE_BOOKMARKS_UI_H__
#define __VINAGRE_BOOKMARKS_UI_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "vinagre-connection.h"
#include "vinagre-bookmarks.h"
#include "vinagre-bookmarks-entry.h"
#include "vinagre-window.h"

G_BEGIN_DECLS

void   vinagre_bookmarks_add        (VinagreBookmarks  *book,
                                     VinagreConnection *conn,
                                     GtkWindow         *window);
void   vinagre_bookmarks_del        (VinagreBookmarks      *book,
                                     VinagreBookmarksEntry *entry,
                                     GtkWindow             *window);
void   vinagre_bookmarks_edit       (VinagreBookmarks      *book,
                                     VinagreBookmarksEntry *entry,
                                     GtkWindow             *window);
void   vinagre_bookmarks_new_folder (VinagreBookmarks      *book,
                                     GtkWindow             *window);

G_END_DECLS

#endif  /* __VINAGRE_BOOKMARKS_UI_H__ */

/* vim: set ts=8: */
