/*
 * vinagre-bookmarks.h
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008  Jonh Wendell <wendell@bani.com.br>
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

#ifndef __VINAGRE_BOOKMARKS_H__
#define __VINAGRE_BOOKMARKS_H__

#include <glib.h>
#include <glib-object.h>

#include "vinagre-connection.h"
#include "vinagre-bookmarks-entry.h"

G_BEGIN_DECLS

#define VINAGRE_BOOKMARKS_FILE      "vinagre-bookmarks.xml"
#define VINAGRE_BOOKMARKS_FILE_OLD  "vinagre.bookmarks"

#define VINAGRE_TYPE_BOOKMARKS             (vinagre_bookmarks_get_type ())
#define VINAGRE_BOOKMARKS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_BOOKMARKS, VinagreBookmarks))
#define VINAGRE_BOOKMARKS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_BOOKMARKS, VinagreBookmarksClass))
#define VINAGRE_IS_BOOKMARKS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_BOOKMARKS))
#define VINAGRE_IS_BOOKMARKS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_BOOKMARKS))
#define VINAGRE_BOOKMARKS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_BOOKMARKS, VinagreBookmarksClass))

typedef struct _VinagreBookmarksClass   VinagreBookmarksClass;
typedef struct _VinagreBookmarks        VinagreBookmarks;
typedef struct _VinagreBookmarksPrivate VinagreBookmarksPrivate;

struct _VinagreBookmarksClass
{
  GObjectClass parent_class;

  /* Signals */
  void (* changed) (VinagreBookmarks *book);
};

struct _VinagreBookmarks
{
  GObject parent_instance;
  VinagreBookmarksPrivate *priv;
};

#include "vinagre-window.h"

GType vinagre_bookmarks_get_type (void) G_GNUC_CONST;

VinagreBookmarks   *vinagre_bookmarks_get_default  (void);
GSList             *vinagre_bookmarks_get_all      (VinagreBookmarks *book);
void                vinagre_bookmarks_save_to_file (VinagreBookmarks *book);
void                vinagre_bookmarks_add_entry    (VinagreBookmarks      *book,
                                                    VinagreBookmarksEntry *entry,
                                                    VinagreBookmarksEntry *parent);
gboolean           vinagre_bookmarks_remove_entry  (VinagreBookmarks      *book,
                                                    VinagreBookmarksEntry *entry);

VinagreConnection  *vinagre_bookmarks_exists  (VinagreBookmarks *book,
                                               const gchar *host,
                                               gint port);

G_END_DECLS
#endif  /* __VINAGRE_BOOKMARKS_H__ */
/* vim: set ts=8: */
