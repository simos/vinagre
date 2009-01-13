/*
 * vinagre-prefs.c
 * This file is part of vinagre
 *
 * Copyright (C) Jonh Wendell 2008 <wendell@bani.com.br>
 * 
 * vinagre-prefs.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * vinagre-prefs.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _VINAGRE_PREFS_H_
#define _VINAGRE_PREFS_H_

#include <glib-object.h>
#include "vinagre-window.h"

G_BEGIN_DECLS

#define VINAGRE_TYPE_PREFS             (vinagre_prefs_get_type ())
#define VINAGRE_PREFS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_PREFS, VinagrePrefs))
#define VINAGRE_PREFS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_PREFS, VinagrePrefsClass))
#define VINAGRE_IS_PREFS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_PREFS))
#define VINAGRE_IS_PREFS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_PREFS))
#define VINAGRE_PREFS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_PREFS, VinagrePrefsClass))

typedef struct _VinagrePrefsClass VinagrePrefsClass;
typedef struct _VinagrePrefs VinagrePrefs;
typedef struct _VinagrePrefsPrivate VinagrePrefsPrivate;

struct _VinagrePrefsClass
{
  GObjectClass parent_class;
};

struct _VinagrePrefs
{
  GObject parent_instance;
  VinagrePrefsPrivate *priv;
};

GType vinagre_prefs_get_type (void) G_GNUC_CONST;

VinagrePrefs	*vinagre_prefs_get_default (void);

void		vinagre_prefs_dialog_show (VinagreWindow *window);
G_END_DECLS

#endif /* _VINAGRE_PREFS_H_ */
/* vim: set ts=8: */
