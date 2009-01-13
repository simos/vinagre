/*
 * vinagre-bookmarks-entry.h
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

#ifndef __VINAGRE_BOOKMARKS_ENTRY_H__
#define __VINAGRE_BOOKMARKS_ENTRY_H__

#include <glib-object.h>
#include "vinagre-connection.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_BOOKMARKS_ENTRY             (vinagre_bookmarks_entry_get_type ())
#define VINAGRE_BOOKMARKS_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_BOOKMARKS_ENTRY, VinagreBookmarksEntry))
#define VINAGRE_BOOKMARKS_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_BOOKMARKS_ENTRY, VinagreBookmarksEntryClass))
#define VINAGRE_IS_BOOKMARKS_ENTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_BOOKMARKS_ENTRY))
#define VINAGRE_IS_BOOKMARKS_ENTRY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_BOOKMARKS_ENTRY))
#define VINAGRE_BOOKMARKS_ENTRY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_BOOKMARKS_ENTRY, VinagreBookmarksEntryClass))

typedef struct _VinagreBookmarksEntryClass   VinagreBookmarksEntryClass;
typedef struct _VinagreBookmarksEntry        VinagreBookmarksEntry;
typedef struct _VinagreBookmarksEntryPrivate VinagreBookmarksEntryPrivate;

typedef enum {
  VINAGRE_BOOKMARKS_ENTRY_NODE_INVALID = 0,
  VINAGRE_BOOKMARKS_ENTRY_NODE_FOLDER,
  VINAGRE_BOOKMARKS_ENTRY_NODE_CONN
} VinagreBookmarksEntryNode;

struct _VinagreBookmarksEntryClass
{
  GObjectClass parent_class;
};

struct _VinagreBookmarksEntry
{
  GObject parent_instance;
  VinagreBookmarksEntryPrivate *priv;
};


GType vinagre_bookmarks_entry_get_type (void) G_GNUC_CONST;

VinagreBookmarksEntry *		vinagre_bookmarks_entry_new_folder  (const gchar *name);
VinagreBookmarksEntry *		vinagre_bookmarks_entry_new_conn    (VinagreConnection *conn);

void				vinagre_bookmarks_entry_add_child   (VinagreBookmarksEntry *entry, VinagreBookmarksEntry *child);
gboolean			vinagre_bookmarks_entry_remove_child(VinagreBookmarksEntry *entry,
								     VinagreBookmarksEntry *child);

GSList *			vinagre_bookmarks_entry_get_children(VinagreBookmarksEntry *entry);

void				vinagre_bookmarks_entry_set_node    (VinagreBookmarksEntry *entry, VinagreBookmarksEntryNode node);
VinagreBookmarksEntryNode	vinagre_bookmarks_entry_get_node    (VinagreBookmarksEntry *entry);

void				vinagre_bookmarks_entry_set_conn    (VinagreBookmarksEntry *entry, VinagreConnection *conn);
VinagreConnection *		vinagre_bookmarks_entry_get_conn    (VinagreBookmarksEntry *entry);

void				vinagre_bookmarks_entry_set_name    (VinagreBookmarksEntry *entry, const gchar *name);
const gchar *			vinagre_bookmarks_entry_get_name    (VinagreBookmarksEntry *entry);

//void				vinagre_bookmarks_entry_set_parent  (VinagreBookmarksEntry *entry, VinagreBookmarksEntry *parent);
VinagreBookmarksEntry *		vinagre_bookmarks_entry_get_parent  (VinagreBookmarksEntry *entry);

gint				vinagre_bookmarks_entry_compare     (VinagreBookmarksEntry *a, VinagreBookmarksEntry *b);
G_END_DECLS

#endif  /* __VINAGRE_BOOKMARKS_ENTRY_H__ */

/* vim: set ts=8: */
