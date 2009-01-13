/*
 * vinagre-notebook.h
 * This file is part of vinagre
 *
 * Copyright (C) 2007 - Jonh Wendell <wendell@bani.com.br>
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
  
#ifndef __VINAGRE_NOTEBOOK_H__
#define __VINAGRE_NOTEBOOK_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define VINAGRE_TYPE_NOTEBOOK		(vinagre_notebook_get_type ())
#define VINAGRE_NOTEBOOK(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), VINAGRE_TYPE_NOTEBOOK, VinagreNotebook))
#define VINAGRE_NOTEBOOK_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), VINAGRE_TYPE_NOTEBOOK, VinagreNotebookClass))
#define VINAGRE_IS_NOTEBOOK(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), VINAGRE_TYPE_NOTEBOOK))
#define VINAGRE_IS_NOTEBOOK_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), VINAGRE_TYPE_NOTEBOOK))
#define VINAGRE_NOTEBOOK_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), VINAGRE_TYPE_NOTEBOOK, VinagreNotebookClass))

/* Private structure type */
typedef struct _VinagreNotebookPrivate	VinagreNotebookPrivate;

/*
 * Main object structure
 */
typedef struct _VinagreNotebook		VinagreNotebook;

#include "vinagre-window.h"
#include "vinagre-tab.h"

struct _VinagreNotebook
{
	GtkNotebook notebook;

	/*< private >*/
        VinagreNotebookPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _VinagreNotebookClass	VinagreNotebookClass;

struct _VinagreNotebookClass
{
  GtkNotebookClass parent_class;

  /* Signals */
  void	 (* tab_added)      (VinagreNotebook *notebook,
			     VinagreTab      *tab);
  void	 (* tab_removed)    (VinagreNotebook *notebook,
			     VinagreTab      *tab);
  void	 (* tab_detached)   (VinagreNotebook *notebook,
			     VinagreTab      *tab);
  void	 (* tabs_reordered) (VinagreNotebook *notebook);
  void	 (* tab_close_request)
			    (VinagreNotebook *notebook,
			     VinagreTab      *tab);
};

/*
 * Public methods
 */
GType		vinagre_notebook_get_type		(void) G_GNUC_CONST;

GtkWidget      *vinagre_notebook_new			(VinagreWindow *window);

void		vinagre_notebook_add_tab		(VinagreNotebook *nb,
							 VinagreTab      *tab,
							 gint           position);

void		vinagre_notebook_remove_tab		(VinagreNotebook *nb,
							 VinagreTab      *tab);

void		vinagre_notebook_remove_all_tabs 	(VinagreNotebook *nb);

void		vinagre_notebook_show_hide_tabs		(VinagreNotebook *nb);
G_END_DECLS

#endif /* __VINAGRE_NOTEBOOK_H__ */
/* vim: set ts=8: */
