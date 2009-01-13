/*
 * vinagre-app.h
 * This file is part of vinagre
 *
 * Copyright (C) 2008 - Jonh Wendell <wendell@bani.com.br>
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

#ifndef __VINAGRE_APP_H__
#define __VINAGRE_APP_H__

#include <gtk/gtk.h>

#include "vinagre-window.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_APP              (vinagre_app_get_type())
#define VINAGRE_APP(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_APP, VinagreApp))
#define VINAGRE_APP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_APP, VinagreAppClass))
#define VINAGRE_IS_APP(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_APP))
#define VINAGRE_IS_APP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_APP))
#define VINAGRE_APP_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_APP, VinagreAppClass))

typedef struct _VinagreAppPrivate VinagreAppPrivate;
typedef struct _VinagreApp VinagreApp;
typedef struct _VinagreAppClass VinagreAppClass;

struct _VinagreApp 
{
  GObject object;
  VinagreAppPrivate *priv;
};


struct _VinagreAppClass 
{
  GObjectClass parent_class;
};


GType		vinagre_app_get_type		(void) G_GNUC_CONST;

VinagreApp 	*vinagre_app_get_default	(void);

VinagreWindow	*vinagre_app_create_window	(VinagreApp  *app,
						 GdkScreen *screen);

const GList	*vinagre_app_get_windows	(VinagreApp *app);
VinagreWindow	*vinagre_app_get_active_window	(VinagreApp *app);
GList		*vinagre_app_get_connections	(VinagreApp *app);

VinagreWindow	*vinagre_app_get_window_in_viewport (VinagreApp *app,
						     GdkScreen  *screen,
						     gint        workspace,
						     gint        viewport_x,
						     gint        viewport_y);

G_END_DECLS

#endif  /* __VINAGRE_APP_H__  */
/* vim: set ts=8: */
