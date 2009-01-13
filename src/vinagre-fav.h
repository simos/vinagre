/*
 * vinagre-fav.h
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

#ifndef __VINAGRE_FAV_H__
#define __VINAGRE_FAV_H__

#include <gtk/gtk.h>
#include "vinagre-bookmarks-entry.h"
#include "vinagre-window.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_FAV              (vinagre_fav_get_type())
#define VINAGRE_FAV(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_FAV, VinagreFav))
#define VINAGRE_FAV_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_FAV, VinagreFav const))
#define VINAGRE_FAV_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_FAV, VinagreFavClass))
#define VINAGRE_IS_FAV(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_FAV))
#define VINAGRE_IS_FAV_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_FAV))
#define VINAGRE_FAV_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_FAV, VinagreFavClass))

typedef struct _VinagreFavPrivate VinagreFavPrivate;
typedef struct _VinagreFav        VinagreFav;
typedef struct _VinagreFavClass   VinagreFavClass;

struct _VinagreFav 
{
  GtkVBox vbox;
  VinagreFavPrivate *priv;
};

struct _VinagreFavClass 
{
  GtkVBoxClass parent_class;

  /* Signals */
  void	(* fav_activated)   (VinagreFav *fav,
			     VinagreBookmarksEntry *entry);

  void	(* fav_selected)    (VinagreFav *fav,
			    VinagreBookmarksEntry *entry);
};

GType 	    vinagre_fav_get_type    (void) G_GNUC_CONST;

GtkWidget   *vinagre_fav_new        (VinagreWindow *window);

gboolean    vinagre_fav_update_list (VinagreFav *fav);

G_END_DECLS

#endif  /* __VINAGRE_FAV_H__  */
