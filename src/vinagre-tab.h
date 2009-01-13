/*
 * vinagre-tab.h
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

#ifndef __VINAGRE_TAB_H__
#define __VINAGRE_TAB_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_TAB              (vinagre_tab_get_type())
#define VINAGRE_TAB(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_TAB, VinagreTab))
#define VINAGRE_TAB_CONST(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), VINAGRE_TYPE_TAB, VinagreTab const))
#define VINAGRE_TAB_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), VINAGRE_TYPE_TAB, VinagreTabClass))
#define VINAGRE_IS_TAB(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), VINAGRE_TYPE_TAB))
#define VINAGRE_IS_TAB_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_TAB))
#define VINAGRE_TAB_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), VINAGRE_TYPE_TAB, VinagreTabClass))
#define VINAGRE_TAB_KEY               "VINAGRE_TAB_KEY"

typedef struct _VinagreTabPrivate VinagreTabPrivate;
typedef struct _VinagreTab        VinagreTab;
typedef struct _VinagreTabClass   VinagreTabClass;

#include "vinagre-connection.h"
#include "vinagre-notebook.h"
#include "vinagre-window.h"

typedef enum
{
  VINAGRE_TAB_STATE_INITIALIZING = 1,
  VINAGRE_TAB_STATE_CONNECTED,
  VINAGRE_TAB_STATE_INVALID
} VinagreTabState;


struct _VinagreTab 
{
  GtkVBox vbox;
  VinagreTabPrivate *priv;
};

struct _VinagreTabClass 
{
  GtkVBoxClass parent_class;

  /* Signals */
  void		(* tab_connected)		(VinagreTab *tab);
  void		(* tab_disconnected)		(VinagreTab *tab);
  void		(* tab_initialized)		(VinagreTab *tab);
};

GType 		  vinagre_tab_get_type		(void) G_GNUC_CONST;

GtkWidget 	  *vinagre_tab_new 		(VinagreConnection *conn,
						 VinagreWindow     *window);

VinagreConnection *vinagre_tab_get_conn		(VinagreTab *tab);

GtkWidget         *vinagre_tab_get_vnc		(VinagreTab *tab);

gchar             *vinagre_tab_get_tooltips     (VinagreTab *tab);

void		  vinagre_tab_set_title		(VinagreTab *tab,
						 const char *title);

void		  vinagre_tab_set_notebook	(VinagreTab *tab,
						 VinagreNotebook *nb);
VinagreNotebook   *vinagre_tab_get_notebook	(VinagreTab *tab);

void		  vinagre_tab_take_screenshot	(VinagreTab *tab);
void		  vinagre_tab_send_ctrlaltdel	(VinagreTab *tab);
void		  vinagre_tab_paste_text	(VinagreTab *tab,
						 const gchar *text);

gboolean	  vinagre_tab_set_scaling	(VinagreTab *tab, gboolean active);
gboolean	  vinagre_tab_get_scaling	(VinagreTab *tab);
void	          vinagre_tab_set_readonly	(VinagreTab *tab, gboolean active);
gboolean	  vinagre_tab_get_readonly	(VinagreTab *tab);

gint		  vinagre_tab_get_original_width  (VinagreTab *tab);
gint		  vinagre_tab_get_original_height (VinagreTab *tab);
void		  vinagre_tab_original_size	  (VinagreTab *tab);

VinagreTabState   vinagre_tab_get_state		(VinagreTab *tab);
VinagreTab	  *vinagre_tab_get_from_connection (VinagreConnection *conn);

gboolean	  vinagre_tab_is_pointer_grab	(VinagreTab *tab);
G_END_DECLS

#endif  /* __VINAGRE_TAB_H__  */
/* vim: set ts=8: */
