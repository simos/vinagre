/*
 * vinagre-bookmarks-tree.h
 * This file is part of vinagre
 *
 * Copyright (C) Jonh Wendell 2008 <wendell@bani.com.br>
 * 
 * vinagre is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * vinagre is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _VINAGRE_BOOKMARKS_TREE_H_
#define _VINAGRE_BOOKMARKS_TREE_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "vinagre-bookmarks-entry.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_BOOKMARKS_TREE             (vinagre_bookmarks_tree_get_type ())
#define VINAGRE_BOOKMARKS_TREE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_BOOKMARKS_TREE, VinagreBookmarksTree))
#define VINAGRE_BOOKMARKS_TREE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_BOOKMARKS_TREE, VinagreBookmarksTreeClass))
#define VINAGRE_IS_BOOKMARKS_TREE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_BOOKMARKS_TREE))
#define VINAGRE_IS_BOOKMARKS_TREE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_BOOKMARKS_TREE))
#define VINAGRE_BOOKMARKS_TREE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_BOOKMARKS_TREE, VinagreBookmarksTreeClass))

typedef struct _VinagreBookmarksTreeClass   VinagreBookmarksTreeClass;
typedef struct _VinagreBookmarksTree        VinagreBookmarksTree;
typedef struct _VinagreBookmarksTreePrivate VinagreBookmarksTreePrivate;

struct _VinagreBookmarksTreeClass
{
  GtkVBoxClass parent_class;
};

struct _VinagreBookmarksTree
{
  GtkVBox parent_instance;
  VinagreBookmarksTreePrivate *priv;
};

GType			vinagre_bookmarks_tree_get_type (void) G_GNUC_CONST;

GtkWidget*		vinagre_bookmarks_tree_new (void);
gboolean		vinagre_bookmarks_tree_select_entry       (VinagreBookmarksTree *tree,
								   VinagreBookmarksEntry *entry);
VinagreBookmarksEntry*	vinagre_bookmarks_tree_get_selected_entry (VinagreBookmarksTree *tree);

G_END_DECLS

#endif /* _VINAGRE_BOOKMARKS_TREE_H_ */

/* vim: set ts=8: */
