/*
 * vinagre-connection.h
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

#ifndef __VINAGRE_CONNECTION_H__
#define __VINAGRE_CONNECTION_H__

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define VINAGRE_TYPE_CONNECTION             (vinagre_connection_get_type ())
#define VINAGRE_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VINAGRE_TYPE_CONNECTION, VinagreConnection))
#define VINAGRE_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VINAGRE_TYPE_CONNECTION, VinagreConnectionClass))
#define VINAGRE_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VINAGRE_TYPE_CONNECTION))
#define VINAGRE_IS_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VINAGRE_TYPE_CONNECTION))
#define VINAGRE_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VINAGRE_TYPE_CONNECTION, VinagreConnectionClass))

typedef struct _VinagreConnectionClass   VinagreConnectionClass;
typedef struct _VinagreConnection        VinagreConnection;
typedef struct _VinagreConnectionPrivate VinagreConnectionPrivate;

typedef enum
{
  VINAGRE_CONNECTION_PROTOCOL_VNC = 1,
  VINAGRE_CONNECTION_PROTOCOL_RDP,
  VINAGRE_CONNECTION_PROTOCOL_INVALID
} VinagreConnectionProtocol;

struct _VinagreConnectionClass
{
  GObjectClass parent_class;
};

struct _VinagreConnection
{
  GObject parent_instance;
  VinagreConnectionPrivate *priv;
};

GType vinagre_connection_get_type (void) G_GNUC_CONST;

VinagreConnection *vinagre_connection_new (void);

VinagreConnectionProtocol vinagre_connection_get_protocol (VinagreConnection *conn);
void		          vinagre_connection_set_protocol (VinagreConnection *conn,
							   VinagreConnectionProtocol protocol);

const gchar*	    vinagre_connection_get_host		(VinagreConnection *conn);
void		    vinagre_connection_set_host		(VinagreConnection *conn,
							 const gchar *host);

gint		    vinagre_connection_get_port		(VinagreConnection *conn);
void		    vinagre_connection_set_port		(VinagreConnection *conn,
							 gint port);

const gchar*	    vinagre_connection_get_username	(VinagreConnection *conn);
void		    vinagre_connection_set_username	(VinagreConnection *conn,
							 const gchar *username);

const gchar*	    vinagre_connection_get_password	(VinagreConnection *conn);
void		    vinagre_connection_set_password	(VinagreConnection *conn,
							 const gchar *password);

const gchar*	    vinagre_connection_get_name         (VinagreConnection *conn);
void		    vinagre_connection_set_name	        (VinagreConnection *conn,
							 const gchar *name);

const gchar*	    vinagre_connection_get_desktop_name	(VinagreConnection *conn);
void		    vinagre_connection_set_desktop_name	(VinagreConnection *conn,
							 const gchar *desktop_name);

gchar*		    vinagre_connection_get_best_name	(VinagreConnection *conn);

VinagreConnection*  vinagre_connection_clone		(VinagreConnection *conn);

VinagreConnection*  vinagre_connection_new_from_string	(const gchar *url, gchar **error_msg, gboolean use_bookmarks);
VinagreConnection*  vinagre_connection_new_from_file	(const gchar *uri, gchar **error_msg, gboolean use_bookmarks);

GdkPixbuf*          vinagre_connection_get_icon	(VinagreConnection *conn);

gboolean	    vinagre_connection_get_view_only	(VinagreConnection *conn);
void		    vinagre_connection_set_view_only	(VinagreConnection *conn,
							 gboolean value);

gboolean	    vinagre_connection_get_scaling	(VinagreConnection *conn);
void		    vinagre_connection_set_scaling	(VinagreConnection *conn,
							 gboolean value);

gboolean	    vinagre_connection_get_fullscreen	(VinagreConnection *conn);
void		    vinagre_connection_set_fullscreen	(VinagreConnection *conn,
							 gboolean value);

gint		    vinagre_connection_get_shared	(VinagreConnection *conn);
void		    vinagre_connection_set_shared	(VinagreConnection *conn,
							 gint value);

gboolean	    vinagre_connection_split_string	(const gchar *uri,
							 gchar **host,
							 gint *port,
							 gchar **error_msg);

gchar*		    vinagre_connection_get_string_rep	(VinagreConnection *conn,
							 gboolean has_protocol);
G_END_DECLS

#endif /* __VINAGRE_CONNECTION_H__  */
/* vim: set ts=8: */
